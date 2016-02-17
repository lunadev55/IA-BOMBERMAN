#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#define CIMA 1
#define BAIXO 3
#define DIREITA 4
#define ESQUERDA 2

//variavel global usada. O ID e um valor passado pelo programa principal que vai identificar se o jogador e o player 1 ou player 2
int id;
//ID do jogo
int ident;

int p1b=0, p1f=0, p2b=0, p2f=0; //info de bonus
int iP = -1, jP = -1; //posição de um objetivo encontrado na BFSafe
int iB = -1, jB = -1; //posição de um objetivo encontrado na BonusFS
int iF = -1, jF = -1; //posição de um objetivo encontrado na P2FS
int iS = -1, jS = -1; //posição de um objetivo encontrado na P2FSB

//esta estrutura guarda as posicoes em i e j num tabuleiro
typedef struct pos {
    int i;
    int j;
} pos;


//vetores de deslocamento. {parado, sobe, esquerda, desce, direita}
int dx[] = {0,-1,0,1,0};
int dy[] = {0,0,-1,0,1};
//baixo, cima, direita, esquerda

typedef struct tabela {
    char str1[3],str2[3];
} tabela;

//mapa a ser lido.
tabela tab[11][13], mapAnt[11][13], rexpmap[11][13], simulexpmap[11][13];
int visited[11][13];
pos parent[11][13];
int bonusDist = 0;
int distP2 = 0;
int distP2B = 0;
int uBombI, uBombJ;
pos curP1, curP2;
char P1[3], P2[3];
char B1[3], B2[3];

//PRINTA MAPA ANTERIOR (ATUAL DA RODADA) NO ARQUIVO
void salvarMapAnt () {
    FILE *matrix;
    matrix = fopen ("DDPmapAnt.txt", "w");
    if (matrix == NULL) return;
    int i,j;
    for(i=0; i<11; i++) {
        for(j=0; j<13; j++) fprintf (matrix, "%s%s ", tab[i][j].str1, tab[i][j].str2);
        fprintf (matrix, "\n");
    }
    fclose (matrix);
}

//PRINTA MAPA DE EXPLOSÃO (não é necessário, só pra teste)
void salvarExpMap () {
    FILE *matrix;
    matrix = fopen ("DDPexpMap.txt", "a");
    if (matrix == NULL) return;
    int i,j;
    for(i=0; i<11; i++) {
        for(j=0; j<13; j++) fprintf (matrix, "%s%s ", rexpmap[i][j].str1, rexpmap[i][j].str2);
        fprintf (matrix, "\n");
    }
    fprintf (matrix, "\n");
    fclose (matrix);
}
void salvarSimulExpMap () {
    FILE *matrix;
    matrix = fopen ("DDPsimulexpMap.txt", "a");
    if (matrix == NULL) return;
    int i,j;
    for(i=0; i<11; i++) {
        for(j=0; j<13; j++) fprintf (matrix, "%s%s ", simulexpmap[i][j].str1, simulexpmap[i][j].str2);
        fprintf (matrix, "\n");
    }
    fprintf (matrix, "\n");
    fclose (matrix);
}

//LÊ ARQUIVO DO MAPA ANTERIOR
void lerMapAnt () {
    FILE *matrix;
    matrix = fopen ("DDPmapAnt.txt", "r");
    if (matrix == NULL) return;
    int i,j;
    for(i=0; i<11; i++) {
        for(j=0; j<13; j++) {
            char temp[5];
            fscanf(matrix, "%s", temp);
            mapAnt[i][j].str1[0] = temp[0];
            mapAnt[i][j].str1[1] = temp[1];
            mapAnt[i][j].str2[0] = temp[2];
            mapAnt[i][j].str2[1] = temp[3];
        }
    }
    fclose (matrix);
}

//PEGA MAPA ATUAL
void leitura() {
    int i, j;
    for(i = 0; i<11; i++) {
        for(j = 0; j<13; j++) {
            char temp[5];
            scanf("%s", temp);
            tab[i][j].str1[0] = temp[0];
            tab[i][j].str1[1] = temp[1];
            tab[i][j].str2[0] = temp[2];
            tab[i][j].str2[1] = temp[3];
        }
    }
}

//funcao retorna a posicao corrente de determinado jogador, cuja string (P1 ou P2) e passada como parametro.
pos* cur_pos() {
    if (strcmp("P1", P1) == 0) {
        strcpy(P2, "P2");
        strcpy(B2, "B2");
        strcpy(B1, "B1");
    }
    else {
        strcpy(P2, "P1");
        strcpy(B2, "B1");
        strcpy(B1, "B2");
    }
    int i,j;
    for(i = 0; i < 11; i++)
        for(j = 0; j < 13; j++) {
            if(strcmp(tab[i][j].str1, P1) == 0) {
                curP1.i = i;
                curP1.j = j;
            }
            if(strcmp(tab[i][j].str1, P2) == 0) {
                curP2.i = i;
                curP2.j = j;
            }
            if(strcmp(tab[i][j].str2, "B3") == 0 || strcmp(tab[i][j].str1, "P3") == 0) {
                curP1.i = i;
                curP1.j = j;
                curP2.i = i;
                curP2.j = j;
            }
        }
}


//funcao que checa se o movimento e valido em determinada posicao
int check(int x, int y) {
    if(x>=0 && x<11 && y>=0 && y<13 && (strcmp(tab[x][y].str2,"--")==0 || strcmp(tab[x][y].str2,"+F")==0 || strcmp(tab[x][y].str2,"+B")==0))
        return 1;
    else
        return 0;
}

//funcao que checa se o movimento e valido no mapa de explosão
int safe(int x, int y) {
    if(x>=0 && x<11 && y>=0 && y<13 && (strcmp(rexpmap[x][y].str2,"--")==0 || strcmp(rexpmap[x][y].str2,"+F")==0 || strcmp(rexpmap[x][y].str2,"+B")==0))
        return 1;
    else
        return 0;
}

//funcao que checa se o movimento e valido no mapa de simulação
int safesim(int x, int y) {
    if(x>=0 && x<11 && y>=0 && y<13 && (strcmp(simulexpmap[x][y].str2,"--")==0 || strcmp(simulexpmap[x][y].str2,"+F")==0 || strcmp(simulexpmap[x][y].str2,"+B")==0))
        return 1;
    else
        return 0;
}

//COMPARA MAPA ANTERIOR COM ATUAL E FAZ CONTAGEM DE BONUS
// +1B +1F +2B +2F
void bonusCount () {
    FILE *matrix;
    matrix = fopen("DDPBonusCount.txt", "r");
    fscanf(matrix, "%d %d %d %d", &p1b, &p1f, &p2b, &p2f);
    fclose(matrix);

    if (strcmp(mapAnt[curP1.i][curP1.j].str2, "+B")==0) p1b++;
    else if (strcmp(mapAnt[curP1.i][curP1.j].str2, "+F")==0) p1f++;
    if (strcmp(mapAnt[curP2.i][curP2.j].str2, "+B")==0) p2b++;
    else if (strcmp(mapAnt[curP2.i][curP2.j].str2, "+F")==0) p2f++;

    matrix = fopen("DDPBonusCount.txt", "w");
    fprintf(matrix, "%d %d %d %d", p1b, p1f, p2b, p2f);
    fclose(matrix);
}
//ATUALIZA NUMERO DE BOMBAS NO ARQUIVO
void bombCount(int bombs) {
    FILE *matrix;
    matrix = fopen("DDPBombCount.txt", "w");
    fprintf(matrix, "%d", bombs);
    fclose(matrix);
}

//RETORNA NUMERO DE BOMBAS
int getBombCount() {
    int bombs = 0;
    FILE *matrix;
    matrix = fopen("DDPBombCount.txt", "r");
    fscanf(matrix, "%d", &bombs);
    fclose(matrix);
    return bombs;
}

//VERIFICA SE JOGADOR ESTÁ PERTO DE DESTRUTÍVEL
int pertoDestr() {
    int k;
    for (k=1; k<5; k++) {
        int i = curP1.i + dx[k];
        int j = curP1.j + dy[k];
        if (i<0 || i>=11 || j<0 || j>=13) continue;
        if(strcmp(tab[i][j].str1, "MM")==0) return 1;
    }
    return 0;
}

//FAZ O MAPA DE EXPLOSÃO
void wexpmap(tabela expmap[11][13]) {
    int i, j, k, m, n;
    for(i = 0; i < 11; i++)
        for(j = 0; j < 13; j++) {
            strcpy(expmap[i][j].str1, tab[i][j].str1);
            strcpy(expmap[i][j].str2, tab[i][j].str2);
        }
    int alcanceP1 = 2 + p1f;
    int alcanceP2 = 2 + p2f;
    for(i = 0; i < 11; i++)
        for(j = 0; j < 13; j++) {
            if(strcmp(expmap[i][j].str2, B1) != 0 && strcmp(expmap[i][j].str2, B2) != 0) continue;
            if(strcmp(expmap[i][j].str2, B1) == 0) { //BOMBA DO JOGADOR 1
                strcpy(expmap[i][j].str1, "FF");
                strcpy(expmap[i][j].str2, "FF");
                //cima
                m = i;
                n = j;
                for (k=0; k<alcanceP1; k++) {
                    m += dx[CIMA];
                    n += dy[CIMA];
                    if (m<0 || m>=11 || n<0 || n>=13) break;
                    if (strcmp(expmap[m][n].str2, B1)==0 || strcmp(expmap[m][n].str2, B2)==0) break;
                    if (strcmp(expmap[m][n].str1, "XX")!=0 && strcmp(expmap[m][n].str2, "DD")!=0) {
                        if (strcmp(expmap[m][n].str1, "MM")==0) {
                            strcpy(expmap[m][n].str1, "FF");
                            strcpy(expmap[m][n].str2, "DD");
                            break;
                        } else {
                            strcpy(expmap[m][n].str1, "FF");
                            strcpy(expmap[m][n].str2, "FF");
                        }
                    } else break;
                }
                //baixo
                m = i;
                n = j;
                for (k=0; k<alcanceP1; k++) {
                    m += dx[BAIXO];
                    n += dy[BAIXO];
                    if (m<0 || m>=11 || n<0 || n>=13) break;
                    if (strcmp(expmap[m][n].str2, B1)==0 || strcmp(expmap[m][n].str2, B2)==0) break;
                    if (strcmp(expmap[m][n].str1, "XX")!=0 && strcmp(expmap[m][n].str2, "DD")!=0) {
                        if (strcmp(expmap[m][n].str1, "MM")==0) {
                            strcpy(expmap[m][n].str1, "FF");
                            strcpy(expmap[m][n].str2, "DD");
                            break;
                        } else {
                            strcpy(expmap[m][n].str1, "FF");
                            strcpy(expmap[m][n].str2, "FF");
                        }
                    } else break;
                }
                //direita
                m = i;
                n = j;
                for (k=0; k<alcanceP1; k++) {
                    m += dx[DIREITA];
                    n += dy[DIREITA];
                    if (m<0 || m>=11 || n<0 || n>=13) break;
                    if (strcmp(expmap[m][n].str2, B1)==0 || strcmp(expmap[m][n].str2, B2)==0) break;
                    if (strcmp(expmap[m][n].str1, "XX")!=0 && strcmp(expmap[m][n].str2, "DD")!=0) {
                        if (strcmp(expmap[m][n].str1, "MM")==0) {
                            strcpy(expmap[m][n].str1, "FF");
                            strcpy(expmap[m][n].str2, "DD");
                            break;
                        } else {
                            strcpy(expmap[m][n].str1, "FF");
                            strcpy(expmap[m][n].str2, "FF");
                        }
                    } else break;
                }
                //esquerda
                m = i;
                n = j;
                for (k=0; k<alcanceP1; k++) {
                    m += dx[ESQUERDA];
                    n += dy[ESQUERDA];
                    if (m<0 || m>=11 || n<0 || n>=13) break;
                    if (strcmp(expmap[m][n].str2, B1)==0 || strcmp(expmap[m][n].str2, B2)==0) break;
                    if (strcmp(expmap[m][n].str1, "XX")!=0 && strcmp(expmap[m][n].str2, "DD")!=0) {
                        if (strcmp(expmap[m][n].str1, "MM")==0) {
                            strcpy(expmap[m][n].str1, "FF");
                            strcpy(expmap[m][n].str2, "DD");
                            break;
                        } else {
                            strcpy(expmap[m][n].str1, "FF");
                            strcpy(expmap[m][n].str2, "FF");
                        }
                    } else break;
                }
            } else if(strcmp(expmap[i][j].str2, B2) == 0) { //BOMBA DO JOGADOR 2
                strcpy(expmap[i][j].str1, "FF");
                strcpy(expmap[i][j].str2, "FF");
                //cima
                m = i;
                n = j;
                for (k=0; k<alcanceP2; k++) {
                    m += dx[CIMA];
                    n += dy[CIMA];
                    if (m<0 || m>=11 || n<0 || n>=13) break;
                    if (strcmp(expmap[m][n].str2, B1)==0 || strcmp(expmap[m][n].str2, B2)==0) break;
                    if (strcmp(expmap[m][n].str1, "XX")!=0 && strcmp(expmap[m][n].str2, "DD")!=0) {
                        if (strcmp(expmap[m][n].str1, "MM")==0) {
                            strcpy(expmap[m][n].str1, "FF");
                            strcpy(expmap[m][n].str2, "DD");
                            break;
                        } else {
                            strcpy(expmap[m][n].str1, "FF");
                            strcpy(expmap[m][n].str2, "FF");
                        }
                    } else break;
                }
                //baixo
                m = i;
                n = j;
                for (k=0; k<alcanceP2; k++) {
                    m += dx[BAIXO];
                    n += dy[BAIXO];
                    if (m<0 || m>=11 || n<0 || n>=13) break;
                    if (strcmp(expmap[m][n].str2, B1)==0 || strcmp(expmap[m][n].str2, B2)==0) break;
                    if (strcmp(expmap[m][n].str1, "XX")!=0 && strcmp(expmap[m][n].str2, "DD")!=0) {
                        if (strcmp(expmap[m][n].str1, "MM")==0) {
                            strcpy(expmap[m][n].str1, "FF");
                            strcpy(expmap[m][n].str2, "DD");
                            break;
                        } else {
                            strcpy(expmap[m][n].str1, "FF");
                            strcpy(expmap[m][n].str2, "FF");
                        }
                    } else break;
                }
                //direita
                m = i;
                n = j;
                for (k=0; k<alcanceP2; k++) {
                    m += dx[DIREITA];
                    n += dy[DIREITA];
                    if (m<0 || m>=11 || n<0 || n>=13) break;
                    if (strcmp(expmap[m][n].str2, B1)==0 || strcmp(expmap[m][n].str2, B2)==0) break;
                    if (strcmp(expmap[m][n].str1, "XX")!=0 && strcmp(expmap[m][n].str2, "DD")!=0) {
                        if (strcmp(expmap[m][n].str1, "MM")==0) {
                            strcpy(expmap[m][n].str1, "FF");
                            strcpy(expmap[m][n].str2, "DD");
                            break;
                        } else {
                            strcpy(expmap[m][n].str1, "FF");
                            strcpy(expmap[m][n].str2, "FF");
                        }
                    } else break;
                }
                //esquerda
                m = i;
                n = j;
                for (k=0; k<alcanceP2; k++) {
                    m += dx[ESQUERDA];
                    n += dy[ESQUERDA];
                    if (m<0 || m>=11 || n<0 || n>=13) break;
                    if (strcmp(expmap[m][n].str2, B1)==0 || strcmp(expmap[m][n].str2, B2)==0) break;
                    if (strcmp(expmap[m][n].str1, "XX")!=0 && strcmp(expmap[m][n].str2, "DD")!=0) {
                        if (strcmp(expmap[m][n].str1, "MM")==0) {
                            strcpy(expmap[m][n].str1, "FF");
                            strcpy(expmap[m][n].str2, "DD");
                            break;
                        } else {
                            strcpy(expmap[m][n].str1, "FF");
                            strcpy(expmap[m][n].str2, "FF");
                        }
                    } else break;
                }
            }
        }
    salvarExpMap();
}

//BUSCA DIREÇÃO PRA LUGAR SEGURO
void BFSafe(pos init, char target[], tabela passada[11][13]) {
    //INITIALIZE
    pos Q[143];
    int m, n;
    for(m=0; m<11; m++) {
        for(n=0; n<13; n++) {
            parent[m][n].i = -1;
            parent[m][n].j = -1;
            if (strcmp(passada[m][n].str1, "XX") == 0 || strcmp(passada[m][n].str1, "MM") == 0
                    || strcmp(passada[m][n].str2, "DD") == 0) visited[m][n]=1;
            else visited[m][n] = 0;
        }
    }
    for(n=0; n<143; n++) {
        Q[n].i = -1;
        Q[n].j = -1;
    }
    Q[0] = init;
    visited[Q[0].i][Q[0].j] = 1;
    int q = 143, y = 0, x = 1, c = 0;

    //BEGIN THE LOOP
    while (q > 0 && y <= x) {
        int i, j;
        i = Q[y].i + 1;
        j = Q[y].j;
        if (i>=0 && j>=0 && i<11 && j<13) {
            if(!visited[i][j]) {
                visited[i][j] = 1;
                parent[i][j] = Q[y];
                if (strcmp(passada[i][j].str1, target) == 0) {
                    iP = i;
                    jP = j;
                    return;
                }
                Q[x].i = i;
                Q[x].j = j;
                x++;
            }
        }
        i = Q[y].i;
        j = Q[y].j + 1;
        if (i>=0 && j>=0 && i<11 && j<13) {
            if(!visited[i][j]) {
                visited[i][j] = 1;
                parent[i][j] = Q[y];
                if (strcmp(passada[i][j].str1, target) == 0) {
                    iP = i;
                    jP = j;
                    return;
                }
                Q[x].i = i;
                Q[x].j = j;
                x++;
            }
        }
        i = Q[y].i - 1;
        j = Q[y].j;
        if (i>=0 && j>=0 && i<11 && j<13) {
            if(!visited[i][j]) {
                visited[i][j] = 1;
                parent[i][j] = Q[y];
                if (strcmp(passada[i][j].str1, target) == 0) {
                    iP = i;
                    jP = j;
                    return;
                }
                Q[x].i = i;
                Q[x].j = j;
                x++;
            }
        }
        i = Q[y].i;
        j = Q[y].j - 1;
        if (i>=0 && j>=0 && i<11 && j<13) {
            if(!visited[i][j]) {
                visited[i][j] = 1;
                parent[i][j] = Q[y];
                if (strcmp(passada[i][j].str1, target) == 0) {
                    iP = i;
                    jP = j;
                    return;
                }
                Q[x].i = i;
                Q[x].j = j;
                x++;
            }
        }
        q--;
        y++;
    }
}

int convDir(pos init, int i, int j) {
    int deltaI = i - init.i;
    int deltaJ = j - init.j;
    if (deltaI==-1 && deltaJ==0) return 1;
    if (deltaI==0 && deltaJ==-1) return 2;
    if (deltaI==1 && deltaJ==0) return 3;
    if (deltaI==0 && deltaJ==1) return 4;
}

int bestDir(pos init, char target[]) {
    BFSafe(init, target, rexpmap);
    //if (iP == -1 && jP == -1) return 0; //fica parado se não tiver como andar
    int i = iP;
    int j = jP;
    //percorre as casas visitadas em ordem reversa, começa do objetivo
    //e vai vendo qual a casa pai até encontrar o início (posição P1)
    while(parent[i][j].i != -1) {
        if (parent[i][j].i == init.i && parent[i][j].j == init.j) {
            return convDir(init, i, j); //retorna a direção a ser andada
        }
        int tempi = i;
        int tempj = j;
        i = parent[i][j].i;
        j = parent[tempi][tempj].j;
    }
}

int bestSimDir(pos init, char target[]) {
    BFSafe(init, target, simulexpmap);
    //if (iP == -1 && jP == -1) return 0; //fica parado se não tiver como andar
    int i = iP;
    int j = jP;
    //percorre as casas visitadas em ordem reversa, começa do objetivo
    //e vai vendo qual a casa pai até encontrar o início (posição P1)
    while(parent[i][j].i != -1) {
        if (parent[i][j].i == init.i && parent[i][j].j == init.j) {
            return convDir(init, i, j); //retorna a direção a ser andada
        }
        int tempi = i;
        int tempj = j;
        i = parent[i][j].i;
        j = parent[tempi][tempj].j;
    }
}

//BUSCA BONUS MAIS PERTO E GRAVA SUA DIREÇÃO
void BonusFS(pos init) {
    //INITIALIZE
    pos Q[143];
    int m, n;
    for(m=0; m<11; m++) {
        for(n=0; n<13; n++) {
            parent[m][n].i = -1;
            parent[m][n].j = -1;
            if (strcmp(rexpmap[m][n].str1, "XX") == 0 || strcmp(rexpmap[m][n].str1, "MM") == 0
                    || strcmp(rexpmap[m][n].str2, "DD") == 0 || strcmp(rexpmap[m][n].str1, "FF") == 0) visited[m][n]=1;
            else visited[m][n] = 0;
        }
    }
    for(n=0; n<143; n++) {
        Q[n].i = -1;
        Q[n].j = -1;
    }
    Q[0] = init;
    visited[Q[0].i][Q[0].j] = 1;
    int q = 143, y = 0, x = 1, c = 0;

    //BEGIN THE LOOP
    while (q > 0 && y <= x) {
        int i, j;
        i = Q[y].i + 1;
        j = Q[y].j;
        if (i>=0 && j>=0 && i<11 && j<13) {
            if(!visited[i][j]) {
                visited[i][j] = 1;
                parent[i][j] = Q[y];
                if (strcmp(rexpmap[i][j].str2, "+F") == 0) {
                    iB = i;
                    jB = j;
                    return;
                }
                Q[x].i = i;
                Q[x].j = j;
                x++;
            }
        }
        i = Q[y].i;
        j = Q[y].j + 1;
        if (i>=0 && j>=0 && i<11 && j<13) {
            if(!visited[i][j]) {
                visited[i][j] = 1;
                parent[i][j] = Q[y];
                if (strcmp(rexpmap[i][j].str2, "+F") == 0) {
                    iB = i;
                    jB = j;
                    return;
                }
                Q[x].i = i;
                Q[x].j = j;
                x++;
            }
        }
        i = Q[y].i - 1;
        j = Q[y].j;
        if (i>=0 && j>=0 && i<11 && j<13) {
            if(!visited[i][j]) {
                visited[i][j] = 1;
                parent[i][j] = Q[y];
                if (strcmp(rexpmap[i][j].str2, "+F") == 0) {
                    iB = i;
                    jB = j;
                    return;
                }
                Q[x].i = i;
                Q[x].j = j;
                x++;
            }
        }
        i = Q[y].i;
        j = Q[y].j - 1;
        if (i>=0 && j>=0 && i<11 && j<13) {
            if(!visited[i][j]) {
                visited[i][j] = 1;
                parent[i][j] = Q[y];
                if (strcmp(rexpmap[i][j].str2, "+F") == 0) {
                    iB = i;
                    jB = j;
                    return;
                }
                Q[x].i = i;
                Q[x].j = j;
                x++;
            }
        }
        q--;
        y++;
    }
}

int getBonusDist(pos init) {
    BonusFS(init);
    if (iB == -1 && jB == -1) return 0;
    int i = iB;
    int j = jB;
    //percorre as casas visitadas em ordem reversa, começa do objetivo
    //e vai vendo qual a casa pai até encontrar o início (posição P1)
    while(parent[i][j].i != -1) {
        bonusDist++;
        if (parent[i][j].i == init.i && parent[i][j].j == init.j) {
            return convDir(init, i, j); //retorna a direção a ser andada
        }
        int tempi = i;
        int tempj = j;
        i = parent[i][j].i;
        j = parent[tempi][tempj].j;
    }
}

//GRAVA DIREÇÃO PRA P2
void P2FS(pos init) {
    //INITIALIZE
    pos Q[143];
    int m, n;
    for(m=0; m<11; m++) {
        for(n=0; n<13; n++) {
            parent[m][n].i = -1;
            parent[m][n].j = -1;
            if (strcmp(tab[m][n].str1, "XX") == 0 || strcmp(tab[m][n].str1, "MM") == 0
                    || strcmp(tab[m][n].str2, "DD") == 0 || strcmp(tab[m][n].str1, "FF") == 0) visited[m][n]=1;
            else visited[m][n] = 0;
        }
    }
    for(n=0; n<143; n++) {
        Q[n].i = -1;
        Q[n].j = -1;
    }
    Q[0] = init;
    visited[Q[0].i][Q[0].j] = 1;
    int q = 143, y = 0, x = 1, c = 0;

    //BEGIN THE LOOP
    while (q > 0 && y <= x) {
        int i, j;
        i = Q[y].i + 1;
        j = Q[y].j;
        if (i>=0 && j>=0 && i<11 && j<13) {
            if(!visited[i][j]) {
                visited[i][j] = 1;
                parent[i][j] = Q[y];
                if (strcmp(tab[i][j].str1, P2) == 0) {
                    iF = i;
                    jF = j;
                    return;
                }
                Q[x].i = i;
                Q[x].j = j;
                x++;
            }
        }
        i = Q[y].i;
        j = Q[y].j + 1;
        if (i>=0 && j>=0 && i<11 && j<13) {
            if(!visited[i][j]) {
                visited[i][j] = 1;
                parent[i][j] = Q[y];
                if (strcmp(tab[i][j].str1, P2) == 0) {
                    iF = i;
                    jF = j;
                    return;
                }
                Q[x].i = i;
                Q[x].j = j;
                x++;
            }
        }
        i = Q[y].i - 1;
        j = Q[y].j;
        if (i>=0 && j>=0 && i<11 && j<13) {
            if(!visited[i][j]) {
                visited[i][j] = 1;
                parent[i][j] = Q[y];
                if (strcmp(tab[i][j].str1, P2) == 0) {
                    iF = i;
                    jF = j;
                    return;
                }
                Q[x].i = i;
                Q[x].j = j;
                x++;
            }
        }
        i = Q[y].i;
        j = Q[y].j - 1;
        if (i>=0 && j>=0 && i<11 && j<13) {
            if(!visited[i][j]) {
                visited[i][j] = 1;
                parent[i][j] = Q[y];
                if (strcmp(tab[i][j].str1, P2) == 0) {
                    iF = i;
                    jF = j;
                    return;
                }
                Q[x].i = i;
                Q[x].j = j;
                x++;
            }
        }
        q--;
        y++;
    }
}

int getP2Dist(pos init) {
    P2FS(init);
    if (iF == -1 && jF == -1) return 0;
    int i = iF;
    int j = jF;
    //percorre as casas visitadas em ordem reversa, começa do objetivo
    //e vai vendo qual a casa pai até encontrar o início (posição P1)
    while(parent[i][j].i != -1) {
        distP2++;
        if (parent[i][j].i == init.i && parent[i][j].j == init.j) {
            return convDir(init, i, j); //retorna a direção a ser andada
        }
        int tempi = i;
        int tempj = j;
        i = parent[i][j].i;
        j = parent[tempi][tempj].j;
    }
}

//GRAVA DIREÇÃO PRA P2 COM BLOQUEIO
void P2FSB(pos init) {
    //INITIALIZE
    pos Q[143];
    int m, n;
    for(m=0; m<11; m++) {
        for(n=0; n<13; n++) {
            parent[m][n].i = -1;
            parent[m][n].j = -1;
            if (strcmp(tab[m][n].str1, "XX") == 0 || strcmp(tab[m][n].str2, "DD") == 0 || strcmp(tab[m][n].str1, "FF") == 0) visited[m][n]=1;
            else visited[m][n] = 0;
        }
    }
    for(n=0; n<143; n++) {
        Q[n].i = -1;
        Q[n].j = -1;
    }
    Q[0] = init;
    visited[Q[0].i][Q[0].j] = 1;
    int q = 143, y = 0, x = 1, c = 0;

    //BEGIN THE LOOP
    while (q > 0 && y <= x) {
        int i, j;
        i = Q[y].i + 1;
        j = Q[y].j;
        if (i>=0 && j>=0 && i<11 && j<13) {
            if(!visited[i][j]) {
                visited[i][j] = 1;
                parent[i][j] = Q[y];
                if (strcmp(tab[i][j].str1, P2) == 0) {
                    iS = i;
                    jS = j;
                    return;
                }
                Q[x].i = i;
                Q[x].j = j;
                x++;
            }
        }
        i = Q[y].i;
        j = Q[y].j + 1;
        if (i>=0 && j>=0 && i<11 && j<13) {
            if(!visited[i][j]) {
                visited[i][j] = 1;
                parent[i][j] = Q[y];
                if (strcmp(tab[i][j].str1, P2) == 0) {
                    iS = i;
                    jS = j;
                    return;
                }
                Q[x].i = i;
                Q[x].j = j;
                x++;
            }
        }
        i = Q[y].i - 1;
        j = Q[y].j;
        if (i>=0 && j>=0 && i<11 && j<13) {
            if(!visited[i][j]) {
                visited[i][j] = 1;
                parent[i][j] = Q[y];
                if (strcmp(tab[i][j].str1, P2) == 0) {
                    iS = i;
                    jS = j;
                    return;
                }
                Q[x].i = i;
                Q[x].j = j;
                x++;
            }
        }
        i = Q[y].i;
        j = Q[y].j - 1;
        if (i>=0 && j>=0 && i<11 && j<13) {
            if(!visited[i][j]) {
                visited[i][j] = 1;
                parent[i][j] = Q[y];
                if (strcmp(tab[i][j].str1, P2) == 0) {
                    iS = i;
                    jS = j;
                    return;
                }
                Q[x].i = i;
                Q[x].j = j;
                x++;
            }
        }
        q--;
        y++;
    }
}

int getP2DistB(pos init) {
    P2FSB(init);
    if (iS == -1 && jS == -1) return 0;
    int i = iS;
    int j = jS;
    //percorre as casas visitadas em ordem reversa, começa do objetivo
    //e vai vendo qual a casa pai até encontrar o início (posição P1)
    while(parent[i][j].i != -1) {
        distP2B++;
        if (parent[i][j].i == init.i && parent[i][j].j == init.j) {
            return convDir(init, i, j); //retorna a direção a ser andada
        }
        int tempi = i;
        int tempj = j;
        i = parent[i][j].i;
        j = parent[tempi][tempj].j;
    }
}

//SALVA POSIÇÃO DE ULTIMA BOMBA COLOCADA
void uBomb() {
    FILE *matrix;
    matrix = fopen ("DDPuBomb.txt", "w");
    if (matrix == NULL) return;
    fprintf (matrix, "%d %d", curP1.i, curP1.j);
    fclose (matrix);
}

//PEGA POSIÇÃO DE ULTIMA BOMBA COLOCADA
void getuBomb () {
    FILE *matrix;
    matrix = fopen ("DDPuBomb.txt", "r");
    if (matrix == NULL) return;
    fscanf(matrix, "%d %d", &uBombI, &uBombJ);
    fclose (matrix);
}

int main(int argc, char *argv[]) { //a assinatura da funcao principal deve ser dessa forma
    FILE* file;
    file = fopen("debugging.txt", "a");
    //convercao dos identificadores
    id = atoi(argv[1]);	//identificador do Jogador
    ident = atoi(argv[2]); //identificador da partida

    srand(time(NULL));

    leitura();
    lerMapAnt();

    if(id==1)strcpy(P1,"P1");
    else strcpy(P1,"P2");

    cur_pos(); // o parametro a ser passado depende se o jogador atual e 1 o 2
    bonusCount(); //deve ser executado antes de se jogar
    getuBomb(); //pega posição de ultima bomba jogada
    wexpmap(rexpmap); //mapa de explosão
    strcpy(tab[curP1.i][curP1.j].str2, B1); //coloca uma bomba para simular
    wexpmap(simulexpmap); //gera mapa de simulação
    strcpy(tab[curP1.i][curP1.j].str2, "--"); //retira a bomba colocada
    salvarSimulExpMap();
    int bombs = getBombCount();
    int bombsMAX = 1 + p1b; //numero máximo atual de bombas
    int bonusDir = 666;
    if (safe(curP1.i, curP1.j)) bonusDir = getBonusDist(curP1); //pega direção para o bonus e ao mesmo tempo a distância até ele
    int P2Dir = 666;
    int P2DirB = 666;
    if (safe(curP1.i, curP1.j)) {
        P2Dir = getP2Dist(curP1); //pega direção para o inimigo e ao mesmo tempo a distância até ele
        P2DirB = getP2DistB(curP1);
    }
    int i, j, cont=0;
    if (!safe(curP1.i, curP1.j)) { //se estiver em perigo, anda na direção do lugar seguro mais próximo
        printf("%d 0 %d 0\n",ident, bestDir(curP1, "--"));
        fprintf(file, "fugiu exp [%d]\n", bonusDist);
    } else if (!bombs && safe(curP1.i, curP1.j) && bonusDist > 0 && bonusDist < 8) { //se estiver perto de bonus, anda uma direção segura até lá
        printf("%d 0 %d 0\n",ident, bonusDir);
        fprintf(file, "foi atras de bonus [%d]\n", bonusDist);
    } else if (!bombs && distP2 > 2 && safe(dx[P2Dir]+curP1.i, dy[P2Dir]+curP1.j)) { //se estiver longe do inimigo, anda em sua direção
        printf("%d 0 %d 0\n",ident, P2Dir);
        fprintf(file, "andando ateh o inimigo | distP2 -> [%d] | P2Dir -> [%d]\n", distP2, P2Dir);
    } else if (!bombs && distP2 > 0 && (curP1.i != uBombI) && (curP1.j != uBombJ)) { //se não tiver bomba e inimigo em mapa de simulação, solta
        //!safesim(curP2.i, curP2.j)
        printf("%d 1 0 0\n",ident);
        //bestSimDir(curP1, "--")
        fprintf(file, "soltou bomba perto do inimigo | distP2 -> [%d] | P2Dir -> [%d]\n", distP2, P2Dir);
        bombs++;
        bombCount(bombs);
        uBomb();
    } else if (!bombs && pertoDestr()) { //se não tiver bomba e estiver perto de um destrutível, solta
        printf("%d 1 0 0\n",ident);
        fprintf(file, "soltou bomba [%d]\n", bonusDist);
        bombs++;
        bombCount(bombs);
        uBomb();
    } else if (safe(curP1.i, curP1.j) && bombs>0) { //se tiver bomba e estiver num lugar seguro, explode
        printf("%d 0 0 1\n",ident);
        fprintf(file, "explodiu bomba [%d]\n", bonusDist);
        bombs--;
        bombCount(bombs);
    } else if (!bombs && safe(dx[P2DirB]+curP1.i, dy[P2DirB]+curP1.j)) { //caso nada, anda em direção à P2 mesmo com bloqueio
        printf("%d 0 %d 0\n",ident, P2DirB);
        fprintf(file, "andou pro mato com P2FSB - %d [%d]\n", cont, P2DirB);
    } else { //caso nada de nada, anda aleatóriamente
        do {
            i = 1 + (rand() % 2) + (rand() % 2) + (rand() % 2);
            j = i;
            cont++;
            if (cont>50) break;
        } while(!safe(dx[i]+curP1.i, dy[j]+curP1.j));
        printf("%d 0 %d 0\n",ident, i);
        fprintf(file, "fez nada - %d [%d]\n", cont, bonusDist);
    }
    salvarMapAnt(); //deve ser salvo no final da rodada
    fclose(file);

    return 0;
}
