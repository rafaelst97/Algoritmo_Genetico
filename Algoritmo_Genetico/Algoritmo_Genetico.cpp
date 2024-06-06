#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <random>
#include <ctime>
#include <climits>
#include <locale.h>

using namespace std;

const int MAX_CIDADES = 20;  // Definindo o n�mero m�ximo de cidades

// Fun��o para ler o arquivo de entrada e transformar os dados em matrizes de dist�ncias
vector<vector<vector<int>>> lerArquivo(const string& nomeArquivo) {
    ifstream file(nomeArquivo); // Abre o arquivo para leitura
    vector<vector<vector<int>>> problemas; // Vetor para armazenar os problemas
    string line; // Vari�vel para armazenar cada linha do arquivo

    while (getline(file, line)) { // L� cada linha do arquivo
        stringstream ss(line); // Cria um stream a partir da linha
        string item; // Vari�vel para armazenar cada item da linha
        vector<int> dados; // Vetor para armazenar os dados da linha

        // Separa os itens da linha por ';' e converte para inteiro
        while (getline(ss, item, ';')) {
            dados.push_back(stoi(item));
        }

        int n_cidades = dados[0]; // N�mero de cidades

        // Verifica��o do n�mero m�ximo de cidades
        if (n_cidades > MAX_CIDADES) {
            cerr << "Erro: N�mero de cidades (" << n_cidades << ") excede o m�ximo permitido (" << MAX_CIDADES << ")." << endl;
            exit(EXIT_FAILURE);
        }

        vector<vector<int>> matrizDistancias(n_cidades, vector<int>(n_cidades, 0)); // Matriz de dist�ncias
        int k = 1;
        for (int i = 0; i < n_cidades; ++i) {
            for (int j = i + 1; j < n_cidades; ++j) {
                matrizDistancias[i][j] = dados[k];
                matrizDistancias[j][i] = dados[k];
                ++k;
            }
        }
        problemas.push_back(matrizDistancias); // Adiciona a matriz de dist�ncias aos problemas
    }
    return problemas;
}

// Fun��o para calcular a dist�ncia total de uma rota
int calcularDistanciaTotal(const vector<int>& rota, const vector<vector<int>>& matrizDistancias) {
    int distanciaTotal = 0;
    for (size_t i = 0; i < rota.size() - 1; ++i) {
        distanciaTotal += matrizDistancias[rota[i]][rota[i + 1]];
    }
    distanciaTotal += matrizDistancias[rota.back()][rota[0]];
    return distanciaTotal;
}

// Fun��o para criar a popula��o inicial de rotas
vector<vector<int>> criarPopulacaoInicial(int tamanhoPopulacao, int n_cidades) {
    vector<vector<int>> populacao(tamanhoPopulacao, vector<int>(n_cidades));
    vector<int> rota(n_cidades);
    for (int i = 0; i < n_cidades; ++i) {
        rota[i] = i; // Inicializa a rota com os �ndices das cidades
    }

    // Cria uma popula��o de rotas aleat�rias
    for (int i = 0; i < tamanhoPopulacao; ++i) {
        shuffle(rota.begin(), rota.end(), mt19937(random_device()()));
        populacao[i] = rota;
    }
    return populacao;
}

// Fun��o para realizar o cruzamento entre duas rotas
vector<int> crossover(const vector<int>& pai1, const vector<int>& pai2) {
    int tamanho = pai1.size();
    vector<int> filho(tamanho, -1);
    int inicio = rand() % tamanho;
    int fim = inicio + (rand() % (tamanho - inicio));

    // Copia uma parte do primeiro pai para o filho
    for (int i = inicio; i <= fim; ++i) {
        filho[i] = pai1[i];
    }

    // Preenche o restante do filho com os genes do segundo pai
    int pos = 0;
    for (int i = 0; i < tamanho; ++i) {
        if (find(filho.begin(), filho.end(), pai2[i]) == filho.end()) {
            while (filho[pos] != -1) {
                ++pos;
            }
            filho[pos] = pai2[i];
        }
    }
    return filho;
}

// Fun��o para realizar a muta��o em uma rota
void mutacao(vector<int>& rota, double taxaMutacao) {
    for (size_t i = 0; i < rota.size(); ++i) {
        if (((double)rand() / RAND_MAX) < taxaMutacao) {
            int j = rand() % rota.size();
            swap(rota[i], rota[j]); // Troca as posi��es de dois genes
        }
    }
}

// Fun��o para selecionar os melhores pais atrav�s de torneio
vector<int> selecaoTorneio(const vector<vector<int>>& populacao, const vector<vector<int>>& matrizDistancias, int tamanhoTorneio) {
    vector<int> melhor;
    int melhorDistancia = INT_MAX;

    // Seleciona os melhores indiv�duos em um torneio
    for (int i = 0; i < tamanhoTorneio; ++i) {
        const vector<int>& individuo = populacao[rand() % populacao.size()];
        int distancia = calcularDistanciaTotal(individuo, matrizDistancias);
        if (distancia < melhorDistancia) {
            melhor = individuo;
            melhorDistancia = distancia;
        }
    }
    return melhor;
}

// Fun��o principal do Algoritmo Gen�tico com crit�rio de parada por estabilidade
pair<vector<int>, int> algoritmoGenetico(const vector<vector<int>>& matrizDistancias, int tamanhoPopulacao = 100, int maxGeracoes = 500, double taxaMutacao = 0.01, int tamanhoTorneio = 5, int criterioEstabilidade = 50) {
    int n_cidades = matrizDistancias.size();
    vector<vector<int>> populacao = criarPopulacaoInicial(tamanhoPopulacao, n_cidades);

    int geracoesSemMelhoria = 0;
    int melhorDistanciaAnterior = INT_MAX;

    for (int geracao = 0; geracao < maxGeracoes; ++geracao) {
        vector<vector<int>> novaPopulacao(tamanhoPopulacao);

        // Gera uma nova popula��o atrav�s de cruzamento e muta��o
        for (int i = 0; i < tamanhoPopulacao; ++i) {
            vector<int> pai1 = selecaoTorneio(populacao, matrizDistancias, tamanhoTorneio);
            vector<int> pai2 = selecaoTorneio(populacao, matrizDistancias, tamanhoTorneio);
            vector<int> filho = crossover(pai1, pai2);
            mutacao(filho, taxaMutacao);
            novaPopulacao[i] = filho;
        }
        populacao = novaPopulacao;

        // Encontra a melhor rota na popula��o atual
        vector<int> melhorRotaAtual;
        int melhorDistanciaAtual = INT_MAX;

        for (const auto& rota : populacao) {
            int distancia = calcularDistanciaTotal(rota, matrizDistancias);
            if (distancia < melhorDistanciaAtual) {
                melhorRotaAtual = rota;
                melhorDistanciaAtual = distancia;
            }
        }

        // Verifica se a melhor dist�ncia melhorou
        if (melhorDistanciaAtual < melhorDistanciaAnterior) {
            melhorDistanciaAnterior = melhorDistanciaAtual;
            geracoesSemMelhoria = 0;
        }
        else {
            geracoesSemMelhoria++;
        }

        // Crit�rio de parada por estabilidade
        if (geracoesSemMelhoria >= criterioEstabilidade) {
            cout << "Parando devido � estabilidade nos resultados ap�s " << geracoesSemMelhoria << " gera��es." << endl;
            break;
        }
    }

    // Encontra a melhor rota na popula��o final
    vector<int> melhorRotaFinal;
    int melhorDistanciaFinal = INT_MAX;

    for (const auto& rota : populacao) {
        int distancia = calcularDistanciaTotal(rota, matrizDistancias);
        if (distancia < melhorDistanciaFinal) {
            melhorRotaFinal = rota;
            melhorDistanciaFinal = distancia;
        }
    }
    return make_pair(melhorRotaFinal, melhorDistanciaFinal);
}

int main() {
    setlocale(LC_ALL, ""); // Configura a localidade para o ambiente padr�o do sistema

    srand(time(0));

    string nomeArquivo = "instrucoes.txt"; // Altere para o caminho correto do seu arquivo de entrada
    vector<vector<vector<int>>> problemas = lerArquivo(nomeArquivo);

    // Para cada problema, executa o Algoritmo Gen�tico e exibe a melhor rota encontrada
    for (size_t i = 0; i < problemas.size(); ++i) {
        pair<vector<int>, int> resultado = algoritmoGenetico(problemas[i]);
        vector<int> melhorRota = resultado.first;
        int melhorDistancia = resultado.second;

        cout << "Problema " << i + 1 << ": Melhor rota: ";
        for (int cidade : melhorRota) {
            cout << (cidade + 1) << " ";
        }
        cout << ", Dist�ncia total: " << melhorDistancia << endl;

        cout << "Detalhes da rota: ";
        for (size_t j = 0; j < melhorRota.size() - 1; ++j) {
            cout << (melhorRota[j] + 1) << " -> " << (melhorRota[j + 1] + 1) << " (" << problemas[i][melhorRota[j]][melhorRota[j + 1]] << "), ";
        }
        cout << (melhorRota.back() + 1) << " -> " << (melhorRota[0] + 1) << " (" << problemas[i][melhorRota.back()][melhorRota[0]] << ")" << endl;
    }

    system("pause");
    return 0;
}
