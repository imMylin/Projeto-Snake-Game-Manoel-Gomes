#include <opencv2/opencv.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Sound.hpp>
#include <fstream>

class Colisao {
public:
    static bool verificaColisao(const cv::Rect& rectA, const std::vector<cv::Rect>& rectsB) {
        for (const auto& rectB : rectsB) {
            if (rectA.x < rectB.x + rectB.width && rectA.x + rectA.width > rectB.x &&
                rectA.y < rectB.y + rectB.height && rectA.y + rectA.height > rectB.y) {
                return true; // Colisão detectada
            }
        }
        return false; // Sem colisão
    }
};

class Score {
public:
    static void salvarMaiorScore(int score) {
        std::ofstream scoreFile("scores.txt");
        if (scoreFile.is_open()) {
            scoreFile << "Maior Score: " << score;
            scoreFile.close();
        }
        else {
            std::cout << "Erro ao salvar o arquivo de scores." << std::endl;
        }
    }

    static int carregarMaiorScore() {
        int highestScore = 0;
        std::ifstream inputFile("scores.txt");
        if (inputFile.is_open()) {
            std::string line;
            std::getline(inputFile, line);
            size_t pos = line.find(":");
            if (pos != std::string::npos) {
                std::string scoreStr = line.substr(pos + 1);
                highestScore = std::stoi(scoreStr);
            }
            inputFile.close();
        }
        else {
            std::cout << "Erro ao abrir o arquivo de scores." << std::endl;
        }
        return highestScore;
    }
};

class Menu {
public:
    static void exibirMenu(int highestScore) {
        cv::Mat menuBackground = cv::imread("Imagens/gomes.png");
        cv::Mat menuFrame = menuBackground.clone();

        cv::putText(menuFrame, "Manoel's Snake Game", cv::Point(menuFrame.cols / 2 - 255, 300), cv::FONT_HERSHEY_SIMPLEX, 1.5, cv::Scalar(255, 255, 255), 2); // Título
        cv::putText(menuFrame, "Aperte Enter para jogar", cv::Point(menuFrame.cols / 2 - 160, 450), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(255, 255, 255), 2); // Instrução para jogar
        cv::putText(menuFrame, "Highest Score: " + std::to_string(highestScore), cv::Point(menuFrame.cols - 230, 50), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(255, 255, 255), 2); // Maior pontuação

        cv::imshow("Menu", menuFrame);
    }
};

class GameOver {
public:
    static void exibirGameOver(int score) {
        cv::Mat gameOver = cv::imread("Imagens/mane.jpg");
        cv::Mat gameOverFrame = gameOver.clone();

        cv::putText(gameOverFrame, "Game Over", cv::Point(gameOverFrame.cols / 2 - 140, 300), cv::FONT_HERSHEY_SIMPLEX, 1.5, cv::Scalar(255, 255, 255), 2); // Título
        cv::putText(gameOverFrame, "Score: " + std::to_string(score), cv::Point(gameOverFrame.cols / 2 - 60, 400), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(255, 255, 255), 2); // Score

        cv::putText(gameOverFrame, "Pressione Enter para jogar novamente", cv::Point(gameOverFrame.cols / 2 - 230, 500), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(255, 255, 255), 2); // Jogar novamente
        cv::putText(gameOverFrame, "Pressione Esc para sair", cv::Point(gameOverFrame.cols / 2 - 160, 550), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(255, 255, 255), 2); // Sair do jogo

        cv::imshow("Game Over", gameOverFrame);
    }
};

class Game {
private:
    cv::VideoCapture cap;
    cv::CascadeClassifier faceCascade;

    int x;
    int y;
    int width;
    int height;

    sf::Sound sound;
    int score = 0;
    int highestScore;

    cv::Mat comida;

public:
    Game() : cap(1), x(0), y(0), width(60), height(60), score(0), highestScore(0) {}

    void run() {
        score = 0;
        if (!cap.isOpened()) {
            std::cout << "Erro ao abrir a webcam." << std::endl;
            return;
        }

        if (!faceCascade.load("Imagens/haarcascade_frontalface_default.xml")) {
            std::cout << "Erro ao carregar o arquivo de cascata para detecção de faces." << std::endl;
            return;
        }

        cv::Mat frame;
        cap.read(frame); // Capturar um frame da webcam

        cv::RNG rng(cv::getTickCount()); // Gerar números aleatórios

        x = rng.uniform(0, frame.cols - 100); // Posição x aleatória
        y = rng.uniform(0, frame.rows - 100); // Posição y aleatória

        // Carregar imagem disponível
        comida = cv::imread("Imagens/caneta.png");
        cv::resize(comida, comida, cv::Size(width, height));

        highestScore = Score::carregarMaiorScore(); //carrega o maior score do arquivo salvo

        Menu::exibirMenu(highestScore);
        bool startGame = false;

        if (cv::waitKey(0) != 13) { // Verifica se a tecla Enter foi pressionada
            startGame = true;
            return;
        }

        cv::destroyWindow("Menu");

        int64_t startTime = cv::getTickCount();

        sf::SoundBuffer buffer;
        if (!buffer.loadFromFile("Imagens/manoel.wav")) {
            std::cout << "Erro ao carregar o arquivo de som." << std::endl;
            return;
        }

        sound.setBuffer(buffer);

        while (true) {
            cap.read(frame);

            std::vector<cv::Rect> faces;
            cv::Mat grayFrame;
            cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

            faceCascade.detectMultiScale(grayFrame, faces, 1.1, 5); // detectar rostos

            if (faces.empty() || !Colisao::verificaColisao(cv::Rect(x, y, width, height), faces)) {
                // Desenhar imagem de comida no frame
                cv::Rect roi(x, y, width, height);
                cv::Mat roiFrame = frame(roi);
                comida.copyTo(roiFrame);
            }
            else {
                x = rng.uniform(0, frame.cols - 100); // gera novas coordenadas aleatórias
                y = rng.uniform(0, frame.rows - 100);

                score++; // aumenta o score
                sound.play();

                if (score > highestScore) {
                    highestScore = score; // Atualiza o maior score
                }
            }

            for (const auto& face : faces) {
                cv::rectangle(frame, face, cv::Scalar(255, 0, 0), 2); // desenha um retângulo ao redor do rosto
            }

            int64_t currentTime = cv::getTickCount(); // Tempo atual
            double elapsedTime = (currentTime - startTime) / cv::getTickFrequency(); // Tempo decorrido em segundos
            int remainingTime = static_cast<int>(60 - elapsedTime); // Tempo restante em segundos

            cv::putText(frame, "Score: " + std::to_string(score), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2); // Mostra o score na tela
            cv::putText(frame, "Time: " + std::to_string(remainingTime), cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2); // Mostra o tempo restante na tela

            cv::imshow("Webcam", frame);

            if (elapsedTime >= 60.0) { // Verifica se o tempo máximo de 60 segundos foi atingido
                break;
            }

            if (cv::waitKey(1) == 27)
                break;
        }

        Score::salvarMaiorScore(highestScore);

        GameOver::exibirGameOver(score);

        while (true) {
            int key = cv::waitKey(0);
            if (key == 13) { // Verifica se a tecla Enter foi pressionada
                cv::destroyWindow("Game Over");
                run();
                break;
            }
            else if (key == 27) { // Verifica se a tecla Esc foi pressionada
                cv::destroyAllWindows();
                break;
            }
        }
    }
};

int main() {
    Game game;
    game.run();

    return 0;
}
