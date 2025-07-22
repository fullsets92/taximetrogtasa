#include <plugin.h>
#include <CMessages.h>
#include <game_sa/CPad.h>
#include <game_sa/CWorld.h>
#include <game_sa/CFileMgr.h>
#include <fstream>
#include <string>
#include <windows.h>
#include <d3dx9.h>
#include "RenderWare.h"
#define MINI_CASE_SENSITIVE
#include "ini.h"


using namespace plugin;

class Taximetro {
public:
	// Criação de variáveis
    bool ligado = false;
    bool mostrarHUD = true;
    float precoBase = 5.0f;
    float bandeiras[3] = { 2.0f, 3.0f, 4.5f };
    int bandeiraAtual = 0;
    float distancia = 0.0f;
    float precoTotal = 0.0f;
    bool corridaAtiva = false;
    bool rodando = false;
    std::string textoEscrever;
    std::string textoEscrever2;
    std::chrono::steady_clock::time_point cronometro1;
    std::chrono::steady_clock::time_point cronometro2;
    std::string statusDisplayDefault = "         ----------";
    std::string statusDisplay = statusDisplayDefault;
    
    
    // Variáveis para .ini
    int keyLigar;
    int keyHUD;
    int keyBandeira;
    int keyIniciar;
    int keyReset;
    int tempo;

	// Para evitar que a ação seja realizada múltiplas vezes por engano
    bool teclaLigarPrev = false;
    bool teclaHUDPrev = false;
    bool teclaBandeiraPrev = false;
    bool teclaAtivoPrev = false;
    bool teclaResetPrev = false;
	bool abriuChatPrev = false;
	bool chatAberto = false;

    Taximetro() {
        CarregarINI();
        precoTotal = precoBase;
        m_pD3DXFont = NULL;
        Events::drawingEvent += [&] { Processar(); };
        Events::initRwEvent.Add(InitFont);
        Events::shutdownRwEvent.Add(DestroyFont);
        Events::d3dLostEvent.Add(DestroyFont);
        Events::d3dResetEvent.Add(InitFont);
    }

    // Basicamente o exemplo do DK22Pac
    static ID3DXFont* m_pD3DXFont;
    static void InitFont() {
        IDirect3DDevice9* device = reinterpret_cast<IDirect3DDevice9*>(RwD3D9GetCurrentD3DDevice());
        if (FAILED(D3DXCreateFontA(device, 36, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, "arial", &m_pD3DXFont)))
        {
            Error("Failed to create D3DX font!");
        }
    }
    static void DestroyFont() {
        if (m_pD3DXFont) {
            m_pD3DXFont->Release();
            m_pD3DXFont = NULL;
        }
    }
    void Draw() {
        if (m_pD3DXFont) {
            RECT rect;
            rect.left = 0;
            rect.top = 0;
            rect.bottom = RsGlobal.maximumHeight;
            rect.right = RsGlobal.maximumWidth;

            RECT outlineRect;
            int outlineSize = 1;
            D3DCOLOR outlineColor = D3DCOLOR_RGBA(0, 0, 0, 255);
            for (int dx = -outlineSize; dx <= outlineSize; ++dx) {
                for (int dy = -outlineSize; dy <= outlineSize; ++dy) {
                    if (dx == 0 && dy == 0) continue;

                    outlineRect = rect;
                    OffsetRect(&outlineRect, dx, dy);

                    m_pD3DXFont->DrawTextA(
                        NULL,
                        textoEscrever.c_str(),
                        -1,
                        &outlineRect,
                        DT_LEFT | DT_VCENTER,
                        outlineColor
                    );
                }
            }

            m_pD3DXFont->DrawTextA(NULL, textoEscrever.c_str(), -1, &rect, DT_LEFT | DT_VCENTER, D3DCOLOR_RGBA(255, 255, 0, 255));
            /*m_pD3DXFont->DrawTextA(NULL, textoEscrever2.c_str(), -1, &rect, DT_LEFT | DT_VCENTER, D3DCOLOR_RGBA(255, 255, 0, 255));*/
        }
    }

	// Carrega o .ini usando a mINI library
    void CarregarINI() {
        mINI::INIFile file("taximetro.SA.ini");
        mINI::INIStructure ini;
        file.read(ini);

        precoBase = std::stof(ini["Taximetro"]["PrecoBase"]);
        bandeiras[0] = std::stof(ini["Taximetro"]["Bandeira1"]);
        bandeiras[1] = std::stof(ini["Taximetro"]["Bandeira2"]);
        bandeiras[2] = std::stof(ini["Taximetro"]["Bandeira3"]);
        tempo = std::stoi(ini["Taximetro"]["Tempo"]);
        keyLigar = std::stoi(ini["Taximetro"]["TeclaLigar"]);
        keyHUD = std::stoi(ini["Taximetro"]["TeclaHUD"]);
        keyBandeira = std::stoi(ini["Taximetro"]["TeclaBandeira"]);
        keyIniciar = std::stoi(ini["Taximetro"]["TeclaIniciar"]);
        keyReset = std::stoi(ini["Taximetro"]["TeclaReset"]);
    }

    void Processar() {

		// Checa se o chat está aberto (É falho, mas é oque temos)
        bool abriuChat = KeyPressed('T');
        if (abriuChat && !abriuChatPrev) {
            chatAberto = true;
        }
        abriuChatPrev = abriuChat;

        bool keyESC = KeyPressed(VK_ESCAPE);
        bool keyENTER = KeyPressed(VK_RETURN);
        if (chatAberto && (keyESC || keyENTER)) {
            chatAberto = false;
        }


        // Liga o sistema (toggle) com K
        bool teclaLigar = KeyPressed(keyLigar);
        if (teclaLigar && !teclaLigarPrev && !chatAberto) {
            ligado = !ligado;
        }
        teclaLigarPrev = teclaLigar;

        
        // Só roda se estiver ligado
        if (ligado) {
            // Mostrar/ocultar HUD com L
            bool teclaHUD = KeyPressed(keyHUD);
            if (teclaHUD && !teclaHUDPrev && !chatAberto) {
                mostrarHUD = !mostrarHUD;
            }
            teclaHUDPrev = teclaHUD;

            // Trocar bandeira com I
            bool teclaBandeira = KeyPressed(keyBandeira);
            if (teclaBandeira && !teclaBandeiraPrev && !chatAberto) {
                if (bandeiraAtual == 2) {
                    bandeiraAtual = 0;
                }
                else {
                    bandeiraAtual++;
                }
                //CMessages::AddMessageJumpQ("Bandeira trocada", 500, 0, false);
            }
            teclaBandeiraPrev = teclaBandeira;

            // Inicia/Para Corrida com O
            bool teclaAtivo = KeyPressed(keyIniciar);
            if (teclaAtivo && !teclaAtivoPrev && !chatAberto) {
                if (!corridaAtiva) {
                    corridaAtiva = !corridaAtiva;
                    rodando = true;
                    //CMessages::AddMessageJumpQ("Corrida iniciada!", 500, 0, false);
                    cronometro1 = std::chrono::steady_clock::now();
                    cronometro2 = cronometro1;
                }
                else
                    if (!rodando) {
                        //CMessages::AddMessageJumpQ("Corrida Reiniciada!", 500, 0, false);
                        rodando = true;
                        cronometro1 = std::chrono::steady_clock::now();
                        cronometro2 = cronometro1;
                        statusDisplay = statusDisplayDefault;
                    }
                    else {
                        //CMessages::AddMessageJumpQ("Corrida Pausada", 500, 0, false);
                        rodando = false;
                        statusDisplay.clear();
                        statusDisplay = "       PAUSADO";
                    }
            }
            teclaAtivoPrev = teclaAtivo;

            // Reseta Corrida com P
            bool teclaReset = KeyPressed(keyReset);
            if (teclaReset && !teclaResetPrev && !chatAberto) {
                corridaAtiva = false;
                rodando = false;
                precoTotal = precoBase;
                statusDisplay = statusDisplayDefault;
            }
            teclaResetPrev = teclaReset;

            // Verifica se o jogador está em um veículo
            CVehicle* vehicle = FindPlayerVehicle(-1, true);
            if (!vehicle) {
                ligado = false;
                //CMessages::AddMessageJumpQ("Taximetro desativado: Voce nao esta em um veiculo!", 3000, 0, false);
                return;
            }
            // Se não estiver pausado, calcula o tempo e vai somando o preço
            else
                if (rodando) {
                    if (std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - cronometro1) >= std::chrono::seconds(tempo)) {
                        cronometro1 = std::chrono::steady_clock::now();
                        cronometro2 = cronometro1;
                        statusDisplay.clear();
                        statusDisplay = statusDisplayDefault;
						precoTotal += bandeiras[bandeiraAtual];
                    }
                    if (std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - cronometro2) >= ((std::chrono::seconds(tempo)) / 10.00)) {
                        if (statusDisplay.length() > 1) {
                            statusDisplay.pop_back();
                        }
                        cronometro2 = std::chrono::steady_clock::now();
                    }
                }
			// Mostra o texto do taxímetro se a HUD estiver ativa
            if (mostrarHUD) {
                Draw();
                textoEscrever = std::format(
                    "       TAXÍMETRO\n    Bandeira {} (${:.1f}) \n        Preço: $ {:.1f}\n  {}",
                    bandeiraAtual + 1, bandeiras[bandeiraAtual], precoTotal, statusDisplay);
                //textoEscrever2 = std::format(
                //    "\n\n\n"
                //    );
            }
        }
    }
} gTaximetro;

ID3DXFont* Taximetro::m_pD3DXFont;