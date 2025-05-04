#include "libmodbus/modbus.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <limits>
#include <ios>  

void showError(const std::string& message) {
    MessageBoxA(NULL, message.c_str(), "Erro", MB_ICONERROR);
}

void showInfo(const std::string& message, const std::string& title = "Informacao") {
    MessageBoxA(NULL, message.c_str(), title.c_str(), MB_ICONINFORMATION);
}

int getIntInput(const std::string& prompt) {
    std::cout << prompt;
    int value;
    while (!(std::cin >> value)) {
        std::cin.clear();
        while (std::cin.get() != '\n');
        std::cout << "Entrada invalida. " << prompt;
    }
    return value;
}

std::string getStringInput(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::cin >> input;
    return input;
}

void readCoils(modbus_t* ctx) {
    int start_addr = getIntInput("Digite o endereco inicial do coil (1-9999): ");
    int num_coils = getIntInput("Digite quantos coils deseja ler (1-2000): ");

    int libmodbus_addr = start_addr - 1;

    if (libmodbus_addr < 0 || num_coils <= 0 || (libmodbus_addr + num_coils) > 9999) {
        showError("Endereco invalido! Use valores entre 1-9999");
        return;
    }

    uint8_t* coils = new uint8_t[num_coils];
    if (modbus_read_bits(ctx, libmodbus_addr, num_coils, coils) == -1) {
        showError("Erro na leitura dos coils!");
    }
    else {
        std::string msg;
        for (int i = 0; i < num_coils; i++) {
            msg += "Coil " + std::to_string(start_addr + i) + ": " +
                std::to_string(coils[i]) + "\n";
        }
        showInfo(msg, "Leitura de Coils");
    }
    delete[] coils;
}

void writeCoil(modbus_t* ctx) {
    int coil_addr = getIntInput("Digite o endereco do coil (1-9999): ");

    int libmodbus_addr = coil_addr - 1;

    if (libmodbus_addr < 0 || libmodbus_addr >= 9999) {
        showError("Endereco invalido! Use valores entre 1-9999");
        return;
    }

    while (true) {
        int value;
        do {
            value = getIntInput("Digite o valor (0 ou 1) ou -1 para sair: ");
        } while (value != 0 && value != 1 && value != -1);

        if (value == -1) {
            showInfo("Saindo da escrita de coil...");
            break;
        }

        // Limpa o buffer antes de escrever
        modbus_flush(ctx);

        if (modbus_write_bit(ctx, libmodbus_addr, value) == -1) {
            showError(std::string("Erro na escrita do coil: ") + modbus_strerror(errno));
        }
        else {
            std::string msg = "Coil " + std::to_string(coil_addr) +
                " definido para: " + std::to_string(value);
            showInfo(msg, "Escrita bem-sucedida");

            Sleep(100);


            uint8_t result;
            if (modbus_read_bits(ctx, libmodbus_addr, 1, &result) != -1) {
                std::string confirm = "Valor atual do coil " +
                    std::to_string(coil_addr) + ": " + std::to_string(result);
                showInfo(confirm, "Confirmacao");
            }
        }
    }
}

int main() {
    std::string ip = getStringInput("Digite o IP do CLP (ex: 127.0.0.1): ");
    int port = getIntInput("Digite a porta TCP (ex: 502): ");

    modbus_t* ctx = modbus_new_tcp(ip.c_str(), port);
    if (!ctx) {
        showError("Falha ao criar contexto Modbus!");
        return -1;
    }

    modbus_set_response_timeout(ctx, 1, 0); 
    modbus_set_debug(ctx, TRUE);

    if (modbus_connect(ctx) == -1) {
        showError(std::string("Falha na conexao Modbus: ") + modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    showInfo("Conexao estabelecida com sucesso!", "Status");

    int choice;
    do {
        std::cout << "\n=== MENU MODBUS ===" << std::endl;
        std::cout << "1. Ler coils" << std::endl;
        std::cout << "2. Escrever em um coil" << std::endl;
        std::cout << "3. Sair" << std::endl;
        choice = getIntInput("Escolha uma opcao: ");

        switch (choice) {
        case 1:
            readCoils(ctx);
            break;
        case 2:
        {
            writeCoil(ctx);

            Sleep(200);
        }
        break;
        case 3:
            showInfo("Encerrando o programa...");
            break;
        default:
            showError("Opcao invalida!");
        }
    } while (choice != 3);

    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}