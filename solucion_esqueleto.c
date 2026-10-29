// Uso:  ./tetris           corre los tests y despues el juego
//       ./tetris --test    corre solo los tests (valgrind --leak-check=full ./tetris --test)

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

// ========= CONSTANTES DEL JUEGO =========
#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 800
#define GRID_ROWS 20
#define GRID_COLS 10
#define CELL_SIZE 35
#define OFFSET_X 75
#define OFFSET_Y 50
#define TICK_RATE 30

const int TETROMINOS[7][4][4][4] = {
    { {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}, {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}}, {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}}, {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}} },
    { {{2,0,0,0},{2,2,2,0},{0,0,0,0},{0,0,0,0}}, {{0,2,2,0},{0,2,0,0},{0,2,0,0},{0,0,0,0}}, {{0,0,0,0},{2,2,2,0},{0,0,2,0},{0,0,0,0}}, {{0,2,0,0},{0,2,0,0},{2,2,0,0},{0,0,0,0}} },
    { {{0,0,3,0},{3,3,3,0},{0,0,0,0},{0,0,0,0}}, {{0,3,0,0},{0,3,0,0},{0,3,3,0},{0,0,0,0}}, {{0,0,0,0},{3,3,3,0},{3,0,0,0},{0,0,0,0}}, {{3,3,0,0},{0,3,0,0},{0,3,0,0},{0,0,0,0}} },
    { {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}} },
    { {{0,5,5,0},{5,5,0,0},{0,0,0,0},{0,0,0,0}}, {{0,5,0,0},{0,5,5,0},{0,0,5,0},{0,0,0,0}}, {{0,0,0,0},{0,5,5,0},{5,5,0,0},{0,0,0,0}}, {{5,0,0,0},{5,5,0,0},{0,5,0,0},{0,0,0,0}} },
    { {{0,6,0,0},{6,6,6,0},{0,0,0,0},{0,0,0,0}}, {{0,6,0,0},{0,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,0,0,0},{6,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,6,0,0},{6,6,0,0},{0,6,0,0},{0,0,0,0}} },
    { {{7,7,0,0},{0,7,7,0},{0,0,0,0},{0,0,0,0}}, {{0,0,7,0},{0,7,7,0},{0,7,0,0},{0,0,0,0}}, {{0,0,0,0},{7,7,0,0},{0,7,7,0},{0,0,0,0}}, {{0,7,0,0},{7,7,0,0},{7,0,0,0},{0,0,0,0}} }
};

SDL_Color colors[8] = {
    {30, 30, 30, 255}, {0, 255, 255, 255}, {0, 0, 255, 255}, {255, 165, 0, 255},
    {255, 255, 0, 255}, {0, 255, 0, 255}, {128, 0, 128, 255}, {255, 0, 0, 255}
};

// ========= ESTRUCTURAS DE DATOS =========

// 1. Nodo para la fila del tablero (Lista Enlazada)
typedef struct RowNode {
    int cells[GRID_COLS]; 
    struct RowNode* next; 
} RowNode;

// 2. Nodo para la cola de próximas piezas (Queue)
typedef struct PieceQueueNode {
    int type; 
    struct PieceQueueNode* next;
} PieceQueueNode;

// 3. Nodo para el sistema de partículas 
typedef struct ParticleNode {
    float x, y;
    float vel_x, vel_y;
    int color;
    int life_timer;
    struct ParticleNode* next;
} ParticleNode;

typedef struct ActivePiece {
    int col, row;
    int type;      
    int rotation;  
    int active;
} ActivePiece;

typedef struct GameBoard {
    RowNode* first_row;             
    PieceQueueNode* queue_head;     
    PieceQueueNode* queue_tail;     
    ParticleNode* first_particle;   
    ActivePiece current_piece;
    int score;
    int tick_timer;
} GameBoard;

// ========= VARIABLES GLOBALES =========
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
GameBoard* game_board = NULL;

// ========= EJERCICIO 1: STRINGS (NO USAR string.h) =========

char* strDuplicate(char* src) { 
    // TODO: Contar los caracteres de src, reservar con malloc (no olvidar el '\0'),
    //       copiar todo y devolver el puntero nuevo.
   
    int cantCar = 0;
    char* i = src;
    char caracter = *i; 

    while (caracter != '\0'){
        cantCar ++;
        i++;
        caracter = *i;
    }

    char* res = (char*)malloc(sizeof(char)*(cantCar + 1)); //la cantidad de caracteres más el 0
    
    char* j = res; //iterador para el nuevo arreglo
    i = src; //iterador para el arreglo viejo
    caracter = *i;

    while(caracter != '\0'){
        *j = caracter;
        j++;

        i++;
        caracter = *i;
    }
    *j = 0;

    return res;
}

int strCompare(char* s1, char* s2) { 
    // TODO: Devolver 0 si son iguales, 1 si s1 < s2, -1 si s1 > s2.
    //       Ojo: es la convencion INVERSA a strcmp.

    //Consideramos que una palabra es menor como un diccionario. 
    //Comparamos caracter por caracter, si son distintos, esto define el orden. En caso contrario, se pasa al siguiente. 
    //Si todo los caracteres son iguales y una termina antes, esa palabra es la menor. 

    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0'){
        if (s1[i] < s2[i]){ 
            return 1;
        }
        if (s1[i] > s2[i]){
            return -1;
        }
        i++;
    }

    //caso si termino por que se termino s1, entonces s2 es mas larga
    if (s1[i] == '\0' && s2[i] != '\0'){
        return 1;
    }
    //caso si termino por que se termino s2, entonces s1 es mas larga
    if (s1[i] != '\0' && s2[i] == '\0'){
        return -1;
    }

    return 0;
}

int len(char* s){ //creamos una funcion len para contar la cantidad de caracteres (sin incluir el '\0').
    int res = 0;
    int i = 0;
    while (s[i] != '\0') {
        res ++;
        i++;
    }
    return res;
}

char* strConcatenate(char* src1, char* src2) { 
    // TODO: Reservar un string nuevo con src1 seguido de src2, y liberar src1 y src2.
    //       Precondicion: src1 y src2 vienen de malloc y son punteros distintos.

    char * res = (char*) malloc (sizeof(char) * (len(src1) + len(src2) + 1)); //voy a querer reservar el espacio de la memoria de la cantidad de caracteres de la primera palabra + la de la segunda y le sumo 1 por el 0 al final
    char* j = res;
    char* i = src1;
    char caracter = *i;
    while (caracter != '\0'){ //recorremos el primer str 
        *j = caracter;
        j++;
        i++;
        caracter = *i;
    }
    free(src1); //liberamos su espacio

    i = src2;
    caracter = *i;
    while (caracter != '\0'){ //recorremos el segundo str
        *j = caracter;
        j++;
        i++;
        caracter = *i;
    }
    *j = 0;

    free(src2); //liberamos su espacio

    return res;
}

// ========= EJERCICIO 2: FUNCIONES A IMPLEMENTAR (EL TP) =========

GameBoard* gameBoardNew() {

    // TODO: Reservar memoria para el GameBoard.
    GameBoard* tablero = (GameBoard*)malloc(sizeof(GameBoard));
    
    // TODO: Usar un bucle para hacer malloc de las 20 filas (RowNode) y conectarlas.
    struct RowNode* current = NULL; 
    for (int i= GRID_ROWS-1; i>= 0; i--){ //voy recorriendo de atras para adelante
        struct RowNode* nuevaRow = (RowNode*)malloc(sizeof(RowNode));
        if (nuevaRow == NULL){ //nos fijamos si hubo espacio para crear una nueva fila
            while(current != NULL){   //voy a querer eliminar las otras filas, asi no queda memoria perdida
                struct RowNode* pre = current->next;  //agarro el siguiente
                free(current); 
                current = pre; //muevo mi current para seguir iterando
            }
            tablero->first_row = NULL;
            return NULL; 
        } 
        for (int j = 0; j<GRID_COLS; j++){ //en el caso que pudimos crear con exito la fila, recorremos el array de sus celdas y ponemos todo en 0
            (nuevaRow->cells)[j] = 0; 
        }
        nuevaRow -> next = current;
        current = nuevaRow;
    }
    
    // TODO: El puntero first_row debe apuntar al primero de esos nodos.
    tablero->first_row = current;

    // TODO: Inicializar queue_head, queue_tail y first_particle en NULL.
    tablero->queue_head = NULL;
    tablero->queue_tail = NULL;
    tablero->first_particle = NULL;
    tablero->score = 0;
    tablero->tick_timer = 0;
    tablero->current_piece.active = 0;

    return tablero;
}

void gameBoardDelete(GameBoard* board) { 
    // TODO: Debe tolerar board == NULL.
    if (board == NULL){
        return; 
    } 

    // TODO: Recorrer y liberar todos los RowNode con free().
    struct RowNode* current = board->first_row; //nos paramos en la primera fila.
    while(current != NULL){  // mientras que no este vacia
        struct RowNode* prox = current->next;  //me guardo la proxima asi no pierdo el dato al liberarlo
        free(current);
        current = prox; //me muevo al siguiente
    }
    board->first_row = NULL;
    
    // TODO: Recorrer y liberar todos los PieceQueueNode.
    struct PieceQueueNode* curPiece = board->queue_head; 
    while (curPiece != board->queue_tail){ //hasta no llegar al final de la lista sigo iterando
        struct PieceQueueNode* prox = curPiece->next; //me guardo la proxima asi no pierdo el dato al liberarlo
        free(curPiece);
        curPiece = prox;
    }
    free(curPiece);
    board->queue_head = NULL;
    board->queue_tail = NULL;

    // TODO: Recorrer y liberar todos los ParticleNode.
    struct ParticleNode* curPar = board->first_particle;  //misma logica del anterior
    while(curPar != NULL){ 
        struct ParticleNode* prox = curPar->next;
        free(curPar);
        curPar = prox;
    }
    board->first_particle = NULL;

    // TODO: Finalmente, liberar la estructura GameBoard.
    free(board); 
}

void gameBoardAddParticle(GameBoard* board, float x, float y, int color) {
    // TODO: Crear un ParticleNode con memoria dinámica.
    ParticleNode* newParticle = (ParticleNode*) malloc (sizeof(ParticleNode));
    // TODO: Inicializar x, y, velocidades aleatorias y un life_timer.
    newParticle->x = x;
    newParticle->y = y;
    newParticle->color = color; 
    newParticle->life_timer =  20 + rand() % 20;
    newParticle-> vel_x = (rand() % 100)/ 20.0f - 2.5f;
    newParticle-> vel_y = (rand() % 50)/ 10.0f - 2.0f;

    // TODO: Agregarlo al principio de la lista enlazada simple de partículas.
    newParticle->next = board->first_particle;
    board->first_particle = newParticle;

}

int gameBoardCheckAndClearLines(GameBoard* board) {
    // TODO: Recorrer la lista de RowNode.
    RowNode* current = board->first_row; //empezamos en la primera fila
    RowNode* pre = NULL; //el anterior es NULL
    
    int cantEliminadas = 0;
    for (int i = 0; i< GRID_ROWS; i++){
        int hay0 = 0;
        // TODO: Si un RowNode tiene todas sus celdas > 0, desenlazarlo y hacer free().
        for (int j = 0; j < GRID_COLS && !hay0; j++){ //recorremos para ver si encontramos algo que valga cero.
            if ((current->cells)[j] == 0){
                hay0 = 1; //encontre algo que vale 0.
            }
        }
        if(!hay0 && current == board->first_row){ //si ninguna vale 0 y estoy parada en la primera fila
            board->first_row = current->next; //la primera fila ahora va a ser la que era la segunda(current->next)
            struct RowNode* borrar = current; //queremos borrar la que es current
            current = current->next;  //me muevo en la lista
            // TODO: Por cada celda destruida, invocar a gameBoardAddParticle().
            gameBoardAddParticle(board, rand()%SCREEN_WIDTH, rand()%SCREEN_HEIGHT, 1+rand()%7);
            free(borrar);
            cantEliminadas ++;

            // TODO: Crear un nuevo RowNode vacío (lleno de ceros).
            struct RowNode* nuevaRow = (RowNode*)malloc(sizeof(RowNode)); 
            for (int k = 0; k<GRID_COLS; k++){
                (nuevaRow->cells)[k] = 0; 
            }
            // TODO: Insertar el nuevo RowNode en la CABEZA del tablero (board->first_row).
            board->first_row = nuevaRow; //ahora queremos que la primera fila sea la nueva
            nuevaRow->next = current; 
            pre = nuevaRow; //ahora el pre es la nueva fila
        }    
        else if(!hay0 && current != board->first_row){ //si ninguna vale 0 y NO estoy parada en la primera fila
            pre->next = current->next; //me voy a querer saltear el que es current
            struct RowNode* borrar = current;
            current = current->next; 

            gameBoardAddParticle(board, rand()%SCREEN_WIDTH, rand()%SCREEN_HEIGHT, 1+rand()%7);
            free(borrar);

            cantEliminadas ++;

            struct RowNode* nuevaRow = (RowNode*)malloc(sizeof(RowNode));
            for (int k = 0; k<GRID_COLS; k++){
                (nuevaRow->cells)[k] = 0; 
            }
            nuevaRow->next = board->first_row; //la proxima fila al la que agregue es la que estaba primera
            board->first_row = nuevaRow; //ahora actualizo que la primera sea la nueva.
        }
        else if (hay0){ //si encontre un 0, me muevo en la lista
            pre = current;
            current = current->next;
        }
    }
    return cantEliminadas;
}

void gameBoardEnqueuePiece(GameBoard* board, int type) {
    // TODO: Crear un PieceQueueNode con el 'type'.
    PieceQueueNode* newPiece = (PieceQueueNode*) malloc (sizeof(PieceQueueNode));
    newPiece->type = type;
    newPiece->next = NULL;

    // TODO: Insertarlo al final de la cola (queue_tail) y actualizar punteros.
    if (board->queue_head == NULL && board->queue_tail == NULL){
        board->queue_head = newPiece;
        board->queue_tail = newPiece;
    }
    else{
        (board->queue_tail)->next = newPiece;
        board->queue_tail = newPiece;
    }
}

int gameBoardDequeuePiece(GameBoard* board, int* out_type) {
    // TODO: Si la cola está vacía, retornar 0.
    if (board->queue_head == NULL && board->queue_tail == NULL){
        return 0;
    }
    
    // TODO: Guardar el type en la variable out_type, liberar el nodo y actualizar el head.
    // TODO: Extraer el primer nodo (queue_head).
    *out_type = (board->queue_head)->type;

    if (board->queue_head == board->queue_tail){
        free(board->queue_head);
        board->queue_head = NULL;
        board->queue_tail = NULL;
    }

    else {
        PieceQueueNode* borrar = board->queue_head;
        board->queue_head = borrar->next; 
        free(borrar);
    }

    return 1; 
}

// ========= LOGICA DE DIBUJADO Y COLISIONES (NO MODIFICAR) =========

RowNode* getRowAt(GameBoard* board, int index) {
    RowNode* curr = board->first_row;
    for(int i = 0; i < index && curr != NULL; i++) curr = curr->next;
    return curr;
}

int canMove(GameBoard* board, int target_col, int target_row, int target_rot) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (TETROMINOS[board->current_piece.type][target_rot][r][c] != 0) {
                int board_col = target_col + c;
                int board_row = target_row + r;
                if (board_col < 0 || board_col >= GRID_COLS || board_row >= GRID_ROWS) return 0;
                if (board_row >= 0) {
                    RowNode* row_node = getRowAt(board, board_row);
                    if (row_node && row_node->cells[board_col] != 0) return 0;
                }
            }
        }
    }
    return 1;
}

void lockPiece(GameBoard* board) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            int val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][r][c];
            if (val != 0) {
                RowNode* row_node = getRowAt(board, board->current_piece.row + r);
                if (row_node) row_node->cells[board->current_piece.col + c] = val;
            }
        }
    }
    board->current_piece.active = 0;
    board->score += gameBoardCheckAndClearLines(board) * 100;
}

// Devuelve 0 si la pieza nueva no entra (GAME OVER), 1 en otro caso.
int spawnNextPiece(GameBoard* board) {
    int type;
    if (gameBoardDequeuePiece(board, &type)) {
        board->current_piece.type = type;
        board->current_piece.rotation = 0;
        board->current_piece.col = GRID_COLS / 2 - 2;
        board->current_piece.row = 0;
        board->current_piece.active = 1;
        gameBoardEnqueuePiece(board, rand() % 7);
        if (!canMove(board, board->current_piece.col, board->current_piece.row, board->current_piece.rotation)) {
            printf("GAME OVER. Puntaje final: %d\n", board->score);
            board->current_piece.active = 0;
            return 0;   // main sale del loop y libera tablero y SDL
        }
    }
    return 1;
}

// Devuelve 0 cuando termina la partida.
int gameBoardUpdate(GameBoard* board) {
    if (!board) return 0;
    if (!board->current_piece.active && !spawnNextPiece(board)) return 0;

    board->tick_timer++;
    if (board->tick_timer >= TICK_RATE) {
        board->tick_timer = 0;
        if (canMove(board, board->current_piece.col, board->current_piece.row + 1, board->current_piece.rotation)) {
            board->current_piece.row++;
        } else {
            lockPiece(board);
        }
    }

    ParticleNode* curr_part = board->first_particle;
    ParticleNode* prev_part = NULL;
    while (curr_part) {
        curr_part->x += curr_part->vel_x;
        curr_part->y += curr_part->vel_y;
        curr_part->vel_y += 0.5f;
        curr_part->life_timer--;

        if (curr_part->life_timer <= 0) {
            ParticleNode* to_delete = curr_part;
            if (prev_part == NULL) {
                board->first_particle = curr_part->next;
                curr_part = board->first_particle;
            } else {
                prev_part->next = curr_part->next;
                curr_part = prev_part->next;
            }
            free(to_delete);
        } else {
            prev_part = curr_part;
            curr_part = curr_part->next;
        }
    }
    return 1;
}

void gameBoardDraw(GameBoard* board) {
    if (!board) return;
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect board_rect = {OFFSET_X - 2, OFFSET_Y - 2, GRID_COLS * CELL_SIZE + 4, GRID_ROWS * CELL_SIZE + 4};
    SDL_RenderDrawRect(renderer, &board_rect);

    RowNode* current_row = board->first_row;
    int r = 0;
    while (current_row) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (current_row->cells[c] > 0) {
                SDL_Color col = colors[current_row->cells[c]];
                SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                SDL_Rect cell_rect = {OFFSET_X + c * CELL_SIZE, OFFSET_Y + r * CELL_SIZE, CELL_SIZE - 1, CELL_SIZE - 1};
                SDL_RenderFillRect(renderer, &cell_rect);
            }
        }
        current_row = current_row->next;
        r++;
    }

    if (board->current_piece.active) {
        for(int pr = 0; pr < 4; pr++) {
            for(int pc = 0; pc < 4; pc++) {
                int cell_val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][pr][pc];
                if (cell_val != 0) {
                    SDL_Color col = colors[cell_val];
                    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                    SDL_Rect p_rect = {OFFSET_X + (board->current_piece.col + pc) * CELL_SIZE, 
                                       OFFSET_Y + (board->current_piece.row + pr) * CELL_SIZE, 
                                       CELL_SIZE - 1, CELL_SIZE - 1};
                    SDL_RenderFillRect(renderer, &p_rect);
                }
            }
        }
    }

    ParticleNode* curr_part = board->first_particle;
    while (curr_part) {
        SDL_Color col = colors[curr_part->color];
        int alpha = curr_part->life_timer * 8;   // se satura: SDL recibe un Uint8
        if (alpha > 255) alpha = 255;
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, alpha);
        SDL_Rect part_rect = {(int)curr_part->x, (int)curr_part->y, 6, 6};
        SDL_RenderFillRect(renderer, &part_rect);
        curr_part = curr_part->next;
    }
    SDL_RenderPresent(renderer);
}

int inicializar() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 0;
    window = SDL_CreateWindow("Tetris Base - TP", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) return 0;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) return 0;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return 1;
}

// ========= TESTS (COMPLETAR) =========

int contarFilas(GameBoard* board) {
    int total = 0;
    RowNode* current = board->first_row;
    while (current != NULL) {
        total++;
        current = current->next;
    }
    return total;
}

int filaVacia(RowNode* fila) {
    for (int i = 0; i < GRID_COLS; i++) {
        if (fila->cells[i] != 0){
            return 0; 
        }
    }
    return 1;  
}

void correrTests() {
    // TODO: Implementar los casos minimos del enunciado. Por ejemplo:
    //  - strDuplicate: "", "x", un string con distintos caracteres.

    char* str1 = "";
    char* dupStr1 = strDuplicate(str1);
    printf("%s \n", str1);
    printf("%s \n", dupStr1);
    free(dupStr1);

    char* str2 = "u";
    char* dupStr2 = strDuplicate(str2);
    printf("%s \n", str2);
    printf("%s \n",dupStr2);
    free(dupStr2);
    
    char* str3 = "hola pablo";
    char* dupStr3 = strDuplicate(str3);
    printf("%s \n",str3);
    printf("%s \n",dupStr3);
    free(dupStr3);

    //  - strCompare: "" vs "", "a" vs "a", y una desigualdad en ambos sentidos (s1/s2 y s2/s1).
    assert(strCompare("", "") == 0);
    assert(strCompare ("a","a") == 0);
    assert(strCompare("ahora", "ayer") == 1);
    assert(strCompare("ayer", "ahora") == -1);

    //  - strConcatenate: "" + "abc" y "hola " + "mundo". Los argumentos deben crearse con
    //    strDuplicate, porque strConcatenate los libera (no pasar literales).

    char* s1 = "";
    char* s2 = "abc";
    char* concate = strConcatenate(strDuplicate(s1),strDuplicate(s2));
    printf("%s \n",s1);
    printf("%s \n",s2);
    printf("%s \n",concate);
    free(concate);

    char* s11 = "holaa";
    char* s22 = "mundo";
    char* concate1 = strConcatenate(strDuplicate(s11),strDuplicate(s22));
    printf("%s \n",s11);
    printf("%s \n",s22);
    printf("%s \n",concate1);    
    free(concate1);
    
    
    //  - Cola: 5 push y 6 pop; el 6to debe devolver 0 y dejar queue_head y queue_tail en NULL.
    //    Verificar tambien que un push posterior funcione.
    GameBoard* board = gameBoardNew();
    gameBoardEnqueuePiece(board, 0);
    gameBoardEnqueuePiece(board, 1);
    gameBoardEnqueuePiece(board, 2);
    gameBoardEnqueuePiece(board, 3);
    gameBoardEnqueuePiece(board, 1);

    int t;
    assert(gameBoardDequeuePiece(board, &t) == 1);  
    assert(t == 0);
    assert(gameBoardDequeuePiece(board, &t) == 1); 
    assert(t == 1);
    assert(gameBoardDequeuePiece(board, &t) == 1); 
    assert(t == 2);
    assert(gameBoardDequeuePiece(board, &t) == 1); 
    assert(t == 3);
    assert(gameBoardDequeuePiece(board, &t) == 1); 
    assert(t == 1);

    assert(gameBoardDequeuePiece(board, &t) == 0);

    gameBoardDelete(board);

    //  - Tablero: llenar la fila de indice 19 (la de abajo), llamar a
    //    gameBoardCheckAndClearLines y comprobar que first_row es la fila nueva vacia y que
    //    la lista sigue teniendo 20 filas.
    GameBoard* tablero = gameBoardNew();
    RowNode* fila = getRowAt(tablero, 19);
    for (int i = 0; i < GRID_COLS; i++) {
        fila->cells[i] = 1;   
    }
    int eliminadas = gameBoardCheckAndClearLines(tablero);

    assert(eliminadas == 1);
    assert(contarFilas(tablero) == 20);
    assert(filaVacia(tablero->first_row) == 1);

    gameBoardDelete(tablero);

    GameBoard* ulitmotablero = gameBoardNew();
    RowNode* fila0 = getRowAt(ulitmotablero, 0);
    RowNode* fila1 = getRowAt(ulitmotablero, 1);

    for (int i = 0; i < GRID_COLS; i++) {
        fila0->cells[i] = 1;   
        fila1->cells[i] = 1;   
    }

    int eliminadas2 = gameBoardCheckAndClearLines(ulitmotablero);

    assert(eliminadas2 == 2);
    assert(contarFilas(ulitmotablero) == 20);

    gameBoardDelete(ulitmotablero);
}

// Compara contra "--test" sin depender de string.h ni del Ejercicio 1.
int esFlagTest(char* arg) {
    char* flag = "--test";
    int i = 0;
    while (arg[i] != '\0' && arg[i] == flag[i]) i++;
    return arg[i] == '\0' && flag[i] == '\0';
}

int main(int argc, char* args[]) {
    srand(time(NULL));

    // Los tests corren antes de SDL. Con --test termina aca, asi valgrind solo
    // reporta la memoria del TP (SDL tiene bloques propios que no se liberan).
    correrTests();
    if (argc > 1 && esFlagTest(args[1])) return 0;

    if (!inicializar()) return 1;

    game_board = gameBoardNew();
    if (game_board == NULL) {
        printf("ADVERTENCIA: Implementa gameBoardNew() para iniciar el juego.\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    for(int i=0; i<3; i++) gameBoardEnqueuePiece(game_board, rand() % 7);

    SDL_Event e;
    int game_over = 0;

    while (!game_over) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) game_over = 1;
            if (e.type == SDL_KEYDOWN && game_board->current_piece.active) {
                if (e.key.keysym.sym == SDLK_LEFT && canMove(game_board, game_board->current_piece.col - 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col--;
                if (e.key.keysym.sym == SDLK_RIGHT && canMove(game_board, game_board->current_piece.col + 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col++;
                if (e.key.keysym.sym == SDLK_DOWN && canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                if (e.key.keysym.sym == SDLK_UP) {
                    int next_rot = (game_board->current_piece.rotation + 1) % 4;
                    if (canMove(game_board, game_board->current_piece.col, game_board->current_piece.row, next_rot)) game_board->current_piece.rotation = next_rot;
                }
                if (e.key.keysym.sym == SDLK_SPACE) {
                     while(canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                     lockPiece(game_board);
                }
            }
        }
        if (!gameBoardUpdate(game_board)) game_over = 1;
        gameBoardDraw(game_board);
        SDL_Delay(16);
    }
    gameBoardDelete(game_board);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
