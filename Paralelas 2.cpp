#include <iostream>
#include <vector>
#include <cmath>  // Para sqrt, necesario para subcuadros de mayor tamaño
#include <chrono> // Para medir el tiempo
#include <thread>
#include <mutex>
#include <queue>
#include <future>
#include <omp.h>
#include <random>
#include <iomanip> // Para formatear la salida



using namespace std;
  // Mutex para gestionar acceso a variables compartidas


void menuPrincipal();
std::mutex mtx;
// Cambiado a std::mutex para evitar ambigüedad
const int N9x9 = 9;
const int N16x16 = 16;
const int N25x25 = 25;
const int NUM_HILOS = 8;




// Función auxiliar para verificar si es seguro colocar un número en una celda específica
bool isSafe(const std::vector<std::vector<int>>& board, int row, int col, int num, int dimension) {
    for (int x = 0; x < dimension; x++) {
        if (board[row][x] == num || board[x][col] == num) return false;
    }
    int subSize = std::sqrt(dimension);
    int startRow = row - row % subSize, startCol = col - col % subSize;
    for (int i = 0; i < subSize; i++) {
        for (int j = 0; j < subSize; j++) {
            if (board[startRow + i][startCol + j] == num) return false;
        }
    }
    return true;
}

// Función para resolver una fila del Sudoku utilizando backtracking
bool resolverFila(std::vector<std::vector<int>>& board, int fila, int dimension) {
    for (int col = 0; col < dimension; col++) {
        if (board[fila][col] == 0) {
            for (int num = 1; num <= dimension; num++) {
                if (isSafe(board, fila, col, num, dimension)) {
                    board[fila][col] = num;
                    if (resolverFila(board, fila, dimension)) return true;
                    board[fila][col] = 0;
                }
            }
            return false;
        }
    }
    return true;
}

// Función que los hilos ejecutarán para resolver filas de manera aleatoria
void resolverSudokuPorFilasAleatorias(std::vector<std::vector<int>>& board) {
    int dimension = board.size();
    std::vector<bool> filasResueltas(dimension, false);
    std::srand(std::time(0));

#pragma omp parallel num_threads(8)
    {
        bool progreso = true;
        while (progreso) {
            progreso = false;
            int fila;

#pragma omp critical
            {
                do {
                    fila = std::rand() % dimension;
                } while (filasResueltas[fila]);
                filasResueltas[fila] = true;
            }

            if (resolverFila(board, fila, dimension)) {
                progreso = true;
#pragma omp critical
                filasResueltas[fila] = true;
                std::cout << "Hilo " << omp_get_thread_num() << " resolvió la fila " << fila << std::endl;
            }
            else {
#pragma omp critical
                filasResueltas[fila] = false;
            }
        }
    }
}

// Función para imprimir el tablero de Sudoku en un formato adecuado
void printBoard(const std::vector<std::vector<int>>& board) {
    int size = board.size();
    int subSize = static_cast<int>(std::sqrt(size)); // Tamaño de cada subcuadrícula

    std::cout << "Tablero de Sudoku resuelto:\n";
    for (int i = 0; i < size; i++) {
        if (i % subSize == 0 && i != 0) {
            std::cout << std::string(size * 3 + subSize - 1, '-') << "\n"; // Línea divisoria
        }
        for (int j = 0; j < size; j++) {
            if (j % subSize == 0 && j != 0) {
                std::cout << " | "; // Separador de subcuadrículas
            }
            std::cout << std::setw(2) << board[i][j] << " ";
        }
        std::cout << "\n";
    }
}

// Función para verificar si es seguro colocar un número en una celda en Sudoku de cualquier tamaño
bool isSafe(int** board, int size, int row, int col, int num) {
    // Verificar la fila
    for (int x = 0; x < size; x++) {
        if (board[row][x] == num) {
            return false;
        }
    }
    // Verificar la columna
    for (int x = 0; x < size; x++) {
        if (board[x][col] == num) {
            return false;
        }
    }
    // Verificar la subcuadrícula
    int subSize = static_cast<int>(std::sqrt(size));
    int startRow = row - row % subSize;
    int startCol = col - col % subSize;
    for (int i = 0; i < subSize; i++) {
        for (int j = 0; j < subSize; j++) {
            if (board[startRow + i][startCol + j] == num) {
                return false;
            }
        }
    }
    return true;
}

// Algoritmo de backtracking con memoria dinámica y poda
bool solveSudoku(int** board, int size, int row, int col) {
    // Si hemos llegado al final del tablero
    if (row == size) return true;
    // Si la columna se sale de los límites, pasa a la siguiente fila
    if (col == size) return solveSudoku(board, size, row + 1, 0);
    // Si la celda ya tiene un valor, pasa a la siguiente
    if (board[row][col] != 0) return solveSudoku(board, size, row, col + 1);

    // Poda: verificar números válidos en la posición actual
    for (int num = 1; num <= size; num++) {
        if (isSafe(board, size, row, col, num)) {
            board[row][col] = num; // Colocar el número provisionalmente
            if (solveSudoku(board, size, row, col + 1)) return true; // Avanza
            board[row][col] = 0; // Backtrack: quitar el número
        }
    }
    return false; // Si no hay ninguna opción válida, se devuelve falso
}

// Función para inicializar un tablero con memoria dinámica
int** initializeBoard(const std::vector<std::vector<int>>& initialBoard) {
    int size = initialBoard.size();
    int** board = new int* [size];
    for (int i = 0; i < size; i++) {
        board[i] = new int[size];
        for (int j = 0; j < size; j++) {
            board[i][j] = initialBoard[i][j];
        }
    }
    return board;
}

// Función para liberar la memoria dinámica del tablero
void freeBoard(int** board, int size) {
    for (int i = 0; i < size; i++) {
        delete[] board[i];
    }
    delete[] board;
}

// Función para imprimir el tablero de Sudoku
void printBoard(int** board, int size) {
    std::cout << "{" << std::endl;
    std::cout << "\t\"board\": [" << std::endl;
    for (int i = 0; i < size; i++) {
        std::cout << "\t\t[";
        for (int j = 0; j < size; j++) {
            std::cout << board[i][j];
            if (j != size - 1) std::cout << ", ";
        }
        std::cout << "]";
        if (i != size - 1) std::cout << ",";
        std::cout << std::endl;
    }
    std::cout << "\t]" << std::endl;
    std::cout << "}" << std::endl;
}

// Función principal para resolver un Sudoku de cualquier tamaño
void resolverSudoku(const std::vector<std::vector<int>>& initialBoard) {
    int size = initialBoard.size();
    int** board = initializeBoard(initialBoard);

    // Imprimir el Sudoku antes de resolverlo
    std::cout << "Sudoku a resolver:" << std::endl;
    printBoard(board, size);

    // Medir el tiempo de resolución
    auto start = std::chrono::high_resolution_clock::now();
    if (solveSudoku(board, size, 0, 0)) {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        // Calcular horas, minutos, segundos y milisegundos
        int hours = duration_ms / 3600000;
        int minutes = (duration_ms % 3600000) / 60000;
        int seconds = (duration_ms % 60000) / 1000;
        int milliseconds = duration_ms % 1000;

        std::cout << "Sudoku resuelto exitosamente en ";
        if (hours > 0) {
            std::cout << hours << " horas, " << minutes << " minutos, " << seconds << " segundos y " << milliseconds << " milisegundos." << std::endl;
        }
        else if (minutes > 0) {
            std::cout << minutes << " minutos, " << seconds << " segundos y " << milliseconds << " milisegundos." << std::endl;
        }
        else if (seconds > 0) {
            std::cout << seconds << " segundos y " << milliseconds << " milisegundos." << std::endl;
        }
        else {
            std::cout << milliseconds << " milisegundos." << std::endl;
        }
        printBoard(board, size);
    }
    else {
        std::cout << "No se pudo resolver el Sudoku." << std::endl;
    }

    freeBoard(board, size);
}


// Función para verificar si es seguro colocar un número en una celda en Sudoku de cualquier tamaño (paralelizada)
bool isSafep(int** board, int size, int row, int col, int num) {
    for (int x = 0; x < size; x++) {
        if (board[row][x] == num || board[x][col] == num)
            return false;
    }
    int subSize = static_cast<int>(std::sqrt(size));
    int startRow = row - row % subSize;
    int startCol = col - col % subSize;
    for (int i = 0; i < subSize; i++) {
        for (int j = 0; j < subSize; j++) {
            if (board[startRow + i][startCol + j] == num)
                return false;
        }
    }
    return true;
}

// Algoritmo de backtracking con memoria dinámica y poda (paralelizado)
bool solveSudokup(int** board, int size, int row, int col) {
    if (row == size) return true;
    if (col == size) return solveSudokup(board, size, row + 1, 0);
    if (board[row][col] != 0) return solveSudokup(board, size, row, col + 1);

    for (int num = 1; num <= size; num++) {
        if (isSafep(board, size, row, col, num)) {
            board[row][col] = num;

            std::vector<std::future<bool>> futures;
            bool solucionEncontrada = false;

            // Crear un pool de hilos para distribuir el trabajo en ramas paralelas
            for (int i = 0; i < NUM_HILOS && !solucionEncontrada; ++i) {
                futures.emplace_back(std::async(std::launch::async, [=, &board, &solucionEncontrada]() {
                    if (solveSudokup(board, size, row, col + 1)) {
                        std::lock_guard<std::mutex> guard(mtx);
                        solucionEncontrada = true;
                        return true;
                    }
                    return false;
                    }));
            }

            for (auto& future : futures) {
                if (future.get()) return true;
            }

            board[row][col] = 0;
        }
    }
    return false;
}

// Función para inicializar un tablero con memoria dinámica
int** initializeBoardp(const std::vector<std::vector<int>>& initialBoard) {
    int size = initialBoard.size();
    int** board = new int* [size];
    for (int i = 0; i < size; i++) {
        board[i] = new int[size];
        for (int j = 0; j < size; j++) {
            board[i][j] = initialBoard[i][j];
        }
    }
    return board;
}

// Función para liberar la memoria dinámica del tablero
void freeBoardp(int** board, int size) {
    for (int i = 0; i < size; i++) {
        delete[] board[i];
    }
    delete[] board;
}

// Función para imprimir el tablero de Sudoku
void printBoardp(int** board, int size) {
    std::cout << "{" << std::endl;
    std::cout << "\t\"board\": [" << std::endl;
    for (int i = 0; i < size; i++) {
        std::cout << "\t\t[";
        for (int j = 0; j < size; j++) {
            std::cout << board[i][j];
            if (j != size - 1) std::cout << ", ";
        }
        std::cout << "]";
        if (i != size - 1) std::cout << ",";
        std::cout << std::endl;
    }
    std::cout << "\t]" << std::endl;
    std::cout << "}" << std::endl;
}


void resolverSudokup(const std::vector<std::vector<int>>& initialBoard) {
    int size = initialBoard.size();
    int** board = initializeBoardp(initialBoard);

    std::cout << "Sudoku a resolver:" << std::endl;
    printBoardp(board, size);

    auto start = std::chrono::high_resolution_clock::now();

    // Usar futuros para asegurar que todos los hilos terminen antes de medir el tiempo
    std::vector<std::future<bool>> futures;
    for (int i = 0; i < 8; ++i) {
        futures.push_back(std::async(std::launch::async, solveSudokup, board, size, 0, 0));
    }
    for (auto& f : futures) {
        f.get(); // Espera a que todos los hilos terminen
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    int hours = duration_ms / 3600000;
    int minutes = (duration_ms % 3600000) / 60000;
    int seconds = (duration_ms % 60000) / 1000;
    int milliseconds = duration_ms % 1000;

    std::cout << "Sudoku resuelto exitosamente en ";
    if (hours > 0) {
        std::cout << hours << " horas, " << minutes << " minutos, " << seconds << " segundos y " << milliseconds << " milisegundos." << std::endl;
    }
    else if (minutes > 0) {
        std::cout << minutes << " minutos, " << seconds << " segundos y " << milliseconds << " milisegundos." << std::endl;
    }
    else if (seconds > 0) {
        std::cout << seconds << " segundos y " << milliseconds << " milisegundos." << std::endl;
    }
    else {
        std::cout << milliseconds << " milisegundos." << std::endl;
    }

    printBoardp(board, size);
    freeBoardp(board, size);
}



// Tablero de Sudoku 25x25 de dificultad media como ejemplo de entrada
std::vector<std::vector<int>> board25x25_dificultad_media = {
        {13, 12,  3,  0,  0,  8,  0, 19,  0, 20, 14,  0, 22,  0,  0,  0,  7, 17,  0, 25, 15,  0, 23,  0, 21},
        {24,  0, 11,  0,  5, 25, 10,  0, 17,  0,  8, 18,  1,  4,  0, 21,  0,  9, 12,  0, 19, 20,  0, 13,  0},
        { 6, 16,  0,  0, 21, 11,  0,  2,  1,  0,  0,  0,  7, 23, 12, 18,  0, 22,  0,  0, 24,  0, 17,  0, 10},
        {20, 18,  0,  4, 15,  0, 12,  0,  9, 13, 10,  0, 17,  0, 19, 16,  5, 23, 24,  3,  0,  6,  0,  8,  1},
        {25,  0,  0,  2, 10, 15,  0,  0,  4,  7, 21,  0, 11,  3, 13,  0, 20,  0, 14,  1,  0, 16,  5, 12,  0},
        {14,  0, 12, 24, 16,  0, 13,  1,  0,  0,  2,  3,  0,  0, 18, 17, 11,  7, 25,  0,  9,  4,  8,  0, 22},
        { 0, 20,  0, 13,  6, 18, 22,  5,  0,  9, 24,  7,  0, 12, 25,  8, 10,  0, 19, 23, 14,  0,  1,  0, 15},
        {17, 23,  5,  3,  4,  0, 25, 12,  0, 15,  0, 11,  8,  0, 22,  9,  0,  0,  6,  0,  0, 18,  0, 20,  7},
        { 9,  0, 21, 25,  0,  0, 16,  0,  8,  0,  5,  0, 10, 20, 14, 22, 18,  0,  3, 13,  6, 24,  0, 23, 12},
        {18,  0,  7, 22,  8,  3,  0, 14,  2, 24,  0, 13,  0,  6,  9,  5, 12,  1, 16,  0, 10,  0, 21, 19, 25},
        {21, 14,  0, 10,  0,  9,  4,  8, 12, 18,  0, 16,  0,  0, 11,  0, 23,  3, 13,  7, 17,  0, 19, 15,  2},
        {22,  0, 13, 11, 24,  2, 20,  0, 16,  0, 12, 19, 25,  1, 10,  0,  8,  0, 21, 18,  0,  3,  6,  7,  0},
        {15,  0, 17,  6,  0,  0,  3, 25, 14, 23, 22,  0, 18,  8,  0,  1,  0, 16,  9, 11,  5, 13,  0, 10,  4},
        { 3,  0, 23, 12, 25,  0, 21, 15, 13,  0,  7,  2, 14,  9,  4, 24, 17, 10,  0,  5, 18,  0, 20,  1, 16},
        { 5,  9, 16,  8, 18,  1,  0, 10, 22, 19,  3,  0, 13, 17, 23,  6,  4, 20,  2, 12, 11, 25, 24,  0, 14},
        { 8,  0, 10, 17, 20, 12, 11,  4, 24, 14,  0, 22,  9,  7,  3,  0,  6, 13, 15,  0,  2,  1,  0, 18,  5},
        { 0,  3,  0, 15, 14, 19,  8, 16, 25,  0, 13,  1, 20, 18,  2, 12,  9, 24,  0, 10,  0, 21,  7, 17, 11},
        { 7, 22,  0, 16, 13,  0,  9,  0,  3,  2, 23,  8, 21,  0, 17, 20, 25,  0,  1, 19, 12,  0, 10,  6, 24},
        { 0, 25,  9,  0, 12, 21,  1,  7,  6, 17,  0, 24,  4, 10, 15,  2, 14,  8,  0, 22, 20, 23,  3,  0, 13},
        { 2,  0,  0, 18,  0, 20, 15, 13,  0, 10, 16,  6, 12, 25,  5,  7,  3,  0, 17,  0,  8, 14, 22,  9, 19},
        { 4, 13, 25,  0, 17, 16, 18,  3,  5,  0, 20, 12,  2, 11,  7,  0, 22,  0,  8,  6,  1,  0, 15, 24, 23},
        { 1,  8, 19,  7, 23,  4,  2,  6, 15,  0, 18, 14, 24, 22,  0, 10, 21,  5,  0,  9,  3, 12,  0, 11, 17},
        {16, 11, 18, 20,  3, 22, 14, 24, 10,  8,  9,  0, 19, 15,  0, 13,  1,  0,  7, 17, 25,  5,  4,  2,  6},
        {10,  5, 24,  0, 22, 17, 19, 20, 11, 12, 25,  4,  6, 13,  1,  3, 15, 18, 23,  2,  0,  7,  9, 14,  8},
        {12,  0, 15, 14,  2, 13, 23,  9,  7,  1, 17,  0,  3,  5,  8,  4, 16, 25,  0, 24, 21,  0, 18, 22, 20}
};

// Llamada a la función `resolverSudoku` pasando el objeto `board25x25_dificultad_media`
void resolver25x25() {
    resolverSudoku(board25x25_dificultad_media);
}
void resolver25x25p() {
    resolverSudoku(board25x25_dificultad_media);
}



// Tablero de Sudoku 16x16 de dificultad media como ejemplo de entrada
std::vector<std::vector<int>> board16x16_dificultad_media = {
    {12, 14,  4,  5, 13,  6,  1,  9,  0,  7, 10,  2,  0,  0, 11, 15},
    {15,  6, 10,  1,  2, 11,  5, 14,  8,  3,  0,  0, 12,  7,  4, 16},
    { 9,  3,  0,  8,  0,  7, 15,  0, 11, 14,  5,  1,  0, 13,  6, 10},
    { 7,  0, 11, 13,  8, 16,  0, 10,  0,  4,  0, 12,  9,  0,  5, 14},
    { 0, 16,  0,  3, 10, 13,  8, 15,  0, 11,  7,  5,  0,  6,  2,  9},
    {13,  7,  0,  9,  1,  5,  2, 11,  3,  0, 14,  0, 15,  0, 12,  8},
    { 8,  5, 14,  0,  0,  9,  0, 12,  0, 13,  0,  0,  7, 10,  1,  3},
    { 1, 10,  2,  0, 16,  0,  7,  3,  0,  9,  8,  6,  0,  4,  0,  5},
    { 0, 13,  0, 16,  7,  0,  6,  2,  4,  0,  3,  8,  1,  5,  9, 11},
    { 5,  0,  9, 12, 11, 15,  0,  0, 13,  6,  0,  7,  0, 14, 10,  4},
    { 2, 11,  0,  0,  5,  4,  9, 13, 10, 15,  1,  0, 16, 12,  0,  7},
    {10,  8,  7,  4, 14,  3,  0,  1,  9,  0, 11, 16, 13,  0, 15,  6},
    { 3, 15,  5, 10,  0,  0, 13, 16,  0,  2,  0, 11,  6,  8,  0, 12},
    { 0,  4,  8,  0,  9, 12, 11,  7,  5,  1,  0,  0, 10, 15, 14, 13},
    { 6,  9, 13,  0,  0,  2, 14,  5,  0,  8, 12, 10,  0,  0, 16,  1},
    {11,  0,  1, 14,  0,  8, 10,  6,  7, 16,  4, 13,  0,  9,  3,  2}
};

// Llamada a la función `resolverSudoku` pasando el objeto `board16x16_dificultad_media`
void resolver16x16() {
    resolverSudoku(board16x16_dificultad_media);
}
void resolver16x16p() {
    resolverSudoku(board16x16_dificultad_media);
}



// Tablero de Sudoku 9x9 de dificultad media como ejemplo de entrada
std::vector<std::vector<int>> board9x9_dificultad_media = {
    {5, 3, 0, 0, 7, 0, 0, 0, 0},
    {6, 0, 0, 1, 9, 5, 0, 0, 0},
    {0, 9, 8, 0, 0, 0, 0, 6, 0},
    {8, 0, 0, 0, 6, 0, 0, 0, 3},
    {4, 0, 0, 8, 0, 3, 0, 0, 1},
    {7, 0, 0, 0, 2, 0, 0, 0, 6},
    {0, 6, 0, 0, 0, 0, 2, 8, 0},
    {0, 0, 0, 4, 1, 9, 0, 0, 5},
    {0, 0, 0, 0, 8, 0, 0, 7, 9}
};

// Llamada a la función `resolverSudoku` pasando el objeto `board9x9_dificultad_media`
void resolver9x9() {
    resolverSudoku(board9x9_dificultad_media);
}
void resolver9x9p() {
    resolverSudoku(board9x9_dificultad_media);
}



// Menú principal
void menuPrincipal() {
    int opcionPrincipal;
    int opcionSudoku;

    while (true) {
        std::cout << "=== MENU PRINCIPAL ===" << std::endl;
        std::cout << "1. Solucionar Sudoku sin paralelizar" << std::endl;
        std::cout << "2. Solucionar Sudoku con técnicas de paralelización (por filas)" << std::endl;
        std::cout << "3. Verificar cantidad de hilos disponibles" << std::endl;
        std::cout << "4. Salir" << std::endl;
        std::cout << "Elija una opción: ";
        std::cin >> opcionPrincipal;

        if (opcionPrincipal == 4) {
            std::cout << "Saliendo del programa..." << std::endl;
            break;
        }

        switch (opcionPrincipal) {
        case 1: {
            std::cout << "\n=== Elija el tamaño del Sudoku ===" << std::endl;
            std::cout << "1. Sudoku 9x9" << std::endl;
            std::cout << "2. Sudoku 16x16" << std::endl;
            std::cout << "3. Sudoku 25x25" << std::endl;
            std::cout << "Elija una opción: ";
            std::cin >> opcionSudoku;

            auto start = std::chrono::high_resolution_clock::now();
            switch (opcionSudoku) {
            case 1:
                resolver9x9p();
                break;
            case 2:
                resolver16x16p();
                break;
            case 3:
                resolver25x25p();
                break;
                // Añadir más casos para 16x16 y 25x25 según sea necesario.
            default:
                std::cout << "Opción no válida." << std::endl;
                continue;
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            std::cout << "Tiempo para resolver el Sudoku: " << duration_ms << " ms" << std::endl;
          
            break;
        }

        case 2: {  // Resolver Sudoku con paralelización por filas aleatorias
            std::cout << "\n=== Elija el tamaño del Sudoku ===" << std::endl;
            std::cout << "1. Sudoku 9x9" << std::endl;
            std::cout << "2. Sudoku 16x16" << std::endl;
            std::cout << "3. Sudoku 25x25" << std::endl;
            std::cout << "Elija una opción: ";
            std::cin >> opcionSudoku;

            auto start = std::chrono::high_resolution_clock::now();
            switch (opcionSudoku) {
            case 1:
                resolverSudokuPorFilasAleatorias(board9x9_dificultad_media);
                printBoard(board9x9_dificultad_media);  // Imprimir Sudoku resuelto
                break;
            case 2:
                resolverSudokuPorFilasAleatorias(board16x16_dificultad_media);
                printBoard(board16x16_dificultad_media);  // Imprimir Sudoku resuelto
                break;
            case 3:
                resolverSudokuPorFilasAleatorias(board25x25_dificultad_media);
                printBoard(board25x25_dificultad_media);  // Imprimir Sudoku resuelto
                break;
            default:
                std::cout << "Opción no válida." << std::endl;
                continue;
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            std::cout << "Tiempo para resolver el Sudoku: " << duration_ms << " ms" << std::endl;
            break;
        }


        case 3: {
            unsigned int numHilos = std::thread::hardware_concurrency();
            if (numHilos == 0) {
                std::cout << "No se pudo determinar el número de hilos disponibles." << std::endl;
            }
            else {
                std::cout << "Número de hilos disponibles: " << numHilos << std::endl;
            }
            break;
        }

        default:
            std::cout << "Opción no válida." << std::endl;
            break;
        }

        std::cout << std::endl;
    }
}

int main() {
    menuPrincipal();
    return 0;
}