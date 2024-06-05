#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <random>
#include <ctime>
#include <climits>

using namespace std;

// Função para ler o arquivo de entrada e transformar os dados em matrizes de distâncias
vector<vector<vector<int>>> lerArquivo(const string& nomeArquivo) {
    ifstream file(nomeArquivo);
    vector<vector<vector<int>>> problemas;
    string line;

    while (getline(file, line)) {
        stringstream ss(line);
        string item;
        vector<int> dados;

        while (getline(ss, item, ';')) {
            dados.push_back(stoi(item));
        }

        int n_cidades = dados[0];
        vector<vector<int>> matrizDistancias(n_cidades, vector<int>(n_cidades, 0));
        int k = 1;
        for (int i = 0; i < n_cidades; ++i) {
            for (int j = i + 1; j < n_cidades; ++j) {
                matrizDistancias[i][j] = dados[k];
                matrizDistancias[j][i] = dados[k];
                ++k;
            }
        }
        problemas.push_back(matrizDistancias);
    }
    return problemas;
}

// Função para calcular a aptidão de uma rota
int calcularDistanciaTotal(const vector<int>& rota, const vector<vector<int>>& matrizDistancias) {
    int distanciaTotal = 0;
    for (size_t i = 0; i < rota.size() - 1; ++i) {
        distanciaTotal += matrizDistancias[rota[i]][rota[i + 1]];
    }
    distanciaTotal += matrizDistancias[rota.back()][rota[0]];
    return distanciaTotal;
}

// Função para criar a população inicial
vector<vector<int>> criarPopulacaoInicial(int tamanhoPopulacao, int n_cidades) {
    vector<vector<int>> populacao(tamanhoPopulacao, vector<int>(n_cidades));
    vector<int> rota(n_cidades);
    for (int i = 0; i < n_cidades; ++i) {
        rota[i] = i; // Inicializa a rota com os índices das cidades
    }

    for (int i = 0; i < tamanhoPopulacao; ++i) {
        shuffle(rota.begin(), rota.end(), mt19937(random_device()()));
        populacao[i] = rota;
    }
    return populacao;
}

// Função para realizar o cruzamento entre duas rotas
vector<int> crossover(const vector<int>& pai1, const vector<int>& pai2) {
    int tamanho = pai1.size();
    vector<int> filho(tamanho, -1);
    int inicio = rand() % tamanho;
    int fim = inicio + (rand() % (tamanho - inicio));

    for (int i = inicio; i <= fim; ++i) {
        filho[i] = pai1[i];
    }

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

// Função para realizar mutação em uma rota
void mutacao(vector<int>& rota, double taxaMutacao) {
    for (size_t i = 0; i < rota.size(); ++i) {
        if (((double)rand() / RAND_MAX) < taxaMutacao) {
            int j = rand() % rota.size();
            swap(rota[i], rota[j]);
        }
    }
}

// Função para selecionar os melhores pais
vector<int> selecaoTorneio(const vector<vector<int>>& populacao, const vector<vector<int>>& matrizDistancias, int tamanhoTorneio) {
    vector<int> melhor;
    int melhorDistancia = INT_MAX;

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

// Função principal do Algoritmo Genético
pair<vector<int>, int> algoritmoGenetico(const vector<vector<int>>& matrizDistancias, int tamanhoPopulacao = 100, int numeroGeracoes = 500, double taxaMutacao = 0.01, int tamanhoTorneio = 5) {
    int n_cidades = matrizDistancias.size();
    vector<vector<int>> populacao = criarPopulacaoInicial(tamanhoPopulacao, n_cidades);

    for (int geracao = 0; geracao < numeroGeracoes; ++geracao) {
        vector<vector<int>> novaPopulacao(tamanhoPopulacao);

        for (int i = 0; i < tamanhoPopulacao; ++i) {
            vector<int> pai1 = selecaoTorneio(populacao, matrizDistancias, tamanhoTorneio);
            vector<int> pai2 = selecaoTorneio(populacao, matrizDistancias, tamanhoTorneio);
            vector<int> filho = crossover(pai1, pai2);
            mutacao(filho, taxaMutacao);
            novaPopulacao[i] = filho;
        }
        populacao = novaPopulacao;
    }

    vector<int> melhorRota;
    int melhorDistancia = INT_MAX;

    for (const auto& rota : populacao) {
        int distancia = calcularDistanciaTotal(rota, matrizDistancias);
        if (distancia < melhorDistancia) {
            melhorRota = rota;
            melhorDistancia = distancia;
        }
    }
    return make_pair(melhorRota, melhorDistancia);
}

int main() {
    srand(time(0));

    string nomeArquivo = "instrucoes.txt"; // Altere para o caminho correto do seu arquivo de entrada
    vector<vector<vector<int>>> problemas = lerArquivo(nomeArquivo);

    for (size_t i = 0; i < problemas.size(); ++i) {
        pair<vector<int>, int> resultado = algoritmoGenetico(problemas[i]);
        vector<int> melhorRota = resultado.first;
        int melhorDistancia = resultado.second;

        cout << "Problema " << i + 1 << ": Melhor rota: ";
        for (int cidade : melhorRota) {
            cout << (cidade + 1) << " ";
        }
        cout << ", Distância total: " << melhorDistancia << endl;

        cout << "Detalhes da rota: ";
        for (size_t j = 0; j < melhorRota.size() - 1; ++j) {
            cout << (melhorRota[j] + 1) << " -> " << (melhorRota[j + 1] + 1) << " (" << problemas[i][melhorRota[j]][melhorRota[j + 1]] << "), ";
        }
        cout << (melhorRota.back() + 1) << " -> " << (melhorRota[0] + 1) << " (" << problemas[i][melhorRota.back()][melhorRota[0]] << ")" << endl;
    }

    return 0;
}
