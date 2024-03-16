//
// Created by Aymane on 10/3/2023.
//
#include <iostream>
#include <boost/process.hpp>
#include <sstream>
#include <cmath>
#include <numeric>

namespace bp = boost::process;

double pEvalForWhite = 30.00;
double pEvalForBlack = -30.00;
const std::string EXEC_PATH = "Engine/stockfish-windows-x86-64.exe";
const std::string DEPTH = "22";
const double OPENING_WEIGHT = 0.20;
const double MIDDLEGAME_WEIGHT = 0.35;
const double ENDGAME_WEIGHT = 0.45;
std::vector<double> evaluationsForWhiteOpening, evaluationsForWhiteMiddle, evaluationsForWhiteEnd;
std::vector<double> evaluationsForBlackOpening, evaluationsForBlackMiddle, evaluationsForBlackEnd;
bp::opstream in;
bp::ipstream out;

// Function that communicates with the child process
void communicateWithChild(const std::string &input) {
    in << "position startpos moves " + input << std::endl;
    in << "go depth " + DEPTH << std::endl;
}

// Function that finds the line containing the cp info
std::string parseOutput() {
    std::string lineIterator, Line;
    while (std::getline(out, lineIterator)) {
        if (lineIterator.find("info") != std::string::npos) {
            Line = lineIterator;
        }
        if (lineIterator.find("bestmove") != std::string::npos) {
            break;
        }
    }
    return Line;
}

std::string readGameState(const std::string &path) {
    return "e2e4 c7c5 f1c4 b8c6 d2d3 d7d6 g1f3 c8g4 h2h3 g4h5 g2g4 h5g6 e1g1 g8f6 g1g2 e7e5 b1c3 f8e7 d1e2 e8g8 g2g3 c6a5 c4b3 a5b3 c2b3 f6d7 h3h4 h7h6 h4h5 g6h7 c3d5 f7f5 e4f5 d7f6 d5e7 d8e7 f1h1 f6d5 e2e4 e7f7 f3h4 d5f6 e4f3 d6d5 h4g6 f8e8 g4g5 e5e4 d3e4 f6e4 g3g4 e4g5 c1g5 h6g5 g4g5 f7f6 g5g4 e8e4 g4g3 f6g5 g3h3 h7g6 f5g6 e4h4";
}

int countToken(const std::string &input) {
    int count = 0;
    std::istringstream iss(input);
    std::string token;
    while (iss >> token) {
            count++;
    }
    return count;
}

void displayClearScreen() {
    std::cout << "\033[2J\033[1;1H";
}

std::string progressBar(int all, int done) {
    std::string bar;
    int progress = ceil((static_cast<double>(done) / static_cast<double>(all)) * 100.00);
    for (int i = 0; i < progress; i++) {
        bar += "=";
    }
    bar += ">";
    int width = 100 - progress;
    for(int i = 0; i < width; i++) {
        bar += " ";
    }
    bar += " |";
    return bar;
}
void storeEvaluation(int moveNumber, double evaluation, bool isWhite) {
    if (moveNumber <= 10) {
        if (isWhite) {
            evaluationsForWhiteOpening.push_back(evaluation);
        } else {
            evaluationsForBlackOpening.push_back(evaluation);
        }
    } else if (moveNumber <= 33) {
        if (isWhite) {
            evaluationsForWhiteMiddle.push_back(evaluation);
        } else {
            evaluationsForBlackMiddle.push_back(evaluation);
        }
    } else {
        if (isWhite) {
            evaluationsForWhiteEnd.push_back(evaluation);
        } else {
            evaluationsForBlackEnd.push_back(evaluation);
        }
    }
}

int main() {
    bp::child child(EXEC_PATH, bp::std_in < in, bp::std_out > out);
    if (!child) {
        std::cerr << "Failed to launch process" << std::endl;
        return 1;
    }
    int moveCount = 0;
    int moveCountForBlack = 0;
    int moveCountForWhite = 0;
    std::string input;
    std::vector<double> evaluationsForWhite, evaluationsForBlack;
    std::cout << "Enter moves: ";
    std::getline(std::cin, input);
    std::istringstream iss1(input);
    std::string Token, moves;
    int count = countToken(input);
    std::cout << "Progress 0%" << std::endl;
    int progress = 0;
    while (iss1 >> Token) {
        ++progress;
        displayClearScreen();
        std::cout << "Progress " <<  ceil((static_cast<double>(progress) / static_cast<double>(count)) * 100.00) << "%"
                  << " " << progressBar(count, progress) << std::endl;
        moves += Token + " ";
        communicateWithChild(moves);
        std::string Line = parseOutput();
        std::string token;
        std::istringstream iss2(Line);
        while (iss2 >> token) {
            if (token == "cp") {
                iss2 >> token;
                moveCount++;
                if (moveCount % 2 == 0) {
                    moveCountForBlack++;
                    pEvalForWhite = std::stod(token);
                    double cEvalForBlack = (1.00 - (abs(pEvalForBlack - (-1.00 * std::stod(token))) / 100.00)) * 100.00;
                    if(cEvalForBlack < 0) {
                        cEvalForBlack = 0;
                    }else{
                        cEvalForBlack = cEvalForBlack;
                    }
                    storeEvaluation(moveCountForBlack, cEvalForBlack, false);
                } else {
                    moveCountForWhite++;
                    pEvalForBlack = std::stod(token);
                    double cEvalForWhite = (1.00 - (abs(pEvalForWhite - (-1.00 * std::stod(token))) / 100.00)) * 100.00;
                    if(cEvalForWhite < 0) {
                        cEvalForWhite = 0;
                    }else{
                        cEvalForWhite = cEvalForWhite;}
                    storeEvaluation(moveCountForWhite, cEvalForWhite, true);                }
                break;
            }
        }
        if (moves == (input + " ")) {
            break;
        }
    }

    double averageForWhiteOpening = evaluationsForWhiteOpening.empty() ? 100.0 :
                                    std::accumulate(evaluationsForWhiteOpening.begin(), evaluationsForWhiteOpening.end(), 0.0) / evaluationsForWhiteOpening.size();

    double averageForWhiteMiddle = evaluationsForWhiteMiddle.empty() ? 100.0 :
                                   std::accumulate(evaluationsForWhiteMiddle.begin(), evaluationsForWhiteMiddle.end(), 0.0) / evaluationsForWhiteMiddle.size();

    double averageForWhiteEnd = evaluationsForWhiteEnd.empty() ? 100.0 :
                                std::accumulate(evaluationsForWhiteEnd.begin(), evaluationsForWhiteEnd.end(), 0.0) / evaluationsForWhiteEnd.size();

    double averageForBlackOpening = evaluationsForBlackOpening.empty() ? 100.0 :
                                    std::accumulate(evaluationsForBlackOpening.begin(), evaluationsForBlackOpening.end(), 0.0) / evaluationsForBlackOpening.size();

    double averageForBlackMiddle = evaluationsForBlackMiddle.empty() ? 100.0 :
                                   std::accumulate(evaluationsForBlackMiddle.begin(), evaluationsForBlackMiddle.end(), 0.0) / evaluationsForBlackMiddle.size();

    double averageForBlackEnd = evaluationsForBlackEnd.empty() ? 100.0 :
                                std::accumulate(evaluationsForBlackEnd.begin(), evaluationsForBlackEnd.end(), 0.0) / evaluationsForBlackEnd.size();


    auto accuracyForWhite = OPENING_WEIGHT * averageForWhiteOpening + MIDDLEGAME_WEIGHT * averageForWhiteMiddle + ENDGAME_WEIGHT * averageForWhiteEnd;
    auto accuracyForBlack = OPENING_WEIGHT * averageForBlackOpening + MIDDLEGAME_WEIGHT * averageForBlackMiddle + ENDGAME_WEIGHT * averageForBlackEnd;
    std::cout << "Accuracy for white: " << accuracyForWhite << std::endl;
    std::cout << "Accuracy for black: " << accuracyForBlack << std::endl;

    return 0;
}
