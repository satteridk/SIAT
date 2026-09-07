Documentação Técnica: S.I.A.T OS (v2.6)
Arquitetura de Sistema e Mecânica Operacional
O S.I.A.T (Simple Information Access Terminal) é um sistema operacional embarcado projetado para o microcontrolador ESP32. Sua arquitetura é baseada em três pilares operacionais projetados para maximizar a estabilidade e a fluidez gráfica:

Máquina de Estados Finita (FSM): O sistema abandona variáveis booleanas sobrepostas em favor de um roteador de estado único (estadoAtual). O ESP32 só executa a lógica do aplicativo que detém o foco (Terminal, Calculadora, Snake ou Popup). Isso impede vazamento de memória e sobreposição de comandos na porta serial, blindando a interface gráfica contra travamentos.

Processamento Dual-Core (FreeRTOS): A interface gráfica e a renderização do display (Core 1) estão fisicamente separadas do barramento de comunicação (Core 0). O sistema utiliza a TarefaBackground para escutar e armazenar dados da porta UART secundária de forma invisível, garantindo que a tela nunca congele enquanto o ESP32 aguarda a resposta do hardware escravo (Raspberry Pi Pico).

Gerenciamento de Buffer Estático: Em vez de instanciar objetos String infinitos que fragmentam a memória Heap, o sistema utiliza matrizes bidimensionais e buffers em C puro (char[]) combinados com ponteiros (strtok) para realizar o parsing de arquivos pesados na velocidade nativa do processador.

Módulo Principal (main.cpp)
Este é o ponto de ignição do sistema e o controlador de tráfego do ciclo de processamento.

setup(): Prepara o hardware básico. Configura a porta USB (115200 bps) para debug e a UART2 (921600 bps) para comunicação de altíssima velocidade com o escravo. Delega a rotina TarefaBackground para o Core 0, inicializa o driver do display ST7789 via SPI, renderiza o logo de boot e desenha a interface limpa inicializando as listas encadeadas do menu.

loop(): Atua como o despachante central da arquitetura FSM através de um bloco switch(estadoAtual). Repassa ciclos de CPU unicamente para a função de entrada correspondente ao aplicativo ativo (ex: processarEntradaTerminal()). Adicionalmente, executa cálculos não bloqueantes como o piscar do cursor do terminal e a animação do letreiro lateral (Marquee).

TarefaBackground(void *pvParameters): Task assíncrona do FreeRTOS travada no Core 0. Vigia permanentemente o buffer de hardware da Serial2. Caso a trava de segurança (uartOcupada) esteja desativada, consome as mensagens recebidas do microcontrolador secundário para evitar o transbordamento do buffer físico da placa.

Camada de Hardware (os_hardware.cpp)
Gerencia interrupções elétricas físicas e o controle de energia do chip.

verificarBotaoFisico(): Varre a porta GPIO 13 buscando transições de energia elétrica de HIGH para LOW. Aplica um cálculo de diferença de tempo (debounce de 250ms) para ignorar ruído elétrico do botão. Ao confirmar um clique válido, salva o estado atual do sistema em estadoAnterior e injeta o sistema no modo POPUP_DESLIGAR. Calcula ativamente a subtração do tempo para disparar o desligamento.

desligarSistema(): Sub-rotina crítica de energia. Renderiza uma mensagem de encerramento, drena a voltagem do pino de retroiluminação da tela e configura o RTC (Real-Time Clock) interno para monitorar a porta GPIO 13. Imediatamente após, envia a CPU, RAM e periféricos para o estado esp_deep_sleep_start(), reduzindo o consumo de energia a microamperes.

Motor do Terminal (os_terminal.cpp)
Subsistema que emula a lógica de um console de texto, traduzindo caracteres brutos em matrizes visuais.

processarEntradaTerminal(): Parser mestre de comandos do usuário. Coleta a String digitada, remove espaços mortos e redireciona ações baseadas em palavras-chave. Transita o estado da FSM (CALC), emite ações locais de interface (CLEAR, UP, DOWN, HELP) ou aciona ferramentas externas acopladas ao painel gráfico.

limparAcentos(): Função de sanitização baseada no método Replace. Inspeciona o input bruto e troca caracteres UTF-8 (como á, ç, ã) pelos seus pares alfanuméricos puros para não quebrar a biblioteca Adafruit GFX, que não possui mapeamento de glifos acentuados.

limparCache(), novaLinhaCache(), adicionarAoCache(): Estrutura de dados responsável pelo histórico na tela. novaLinhaCache empurra todo o buffer para cima (descartando a primeira posição) se o limite de linhas for atingido. adicionarAoCache grava o caractere e sua cor exata de forma posicional no array 2D.

desenharLinhaCache(), restaurarPaginaAtual(), renderizarScroll(): Funções de plotagem gráfica. Calculam a quantidade de linhas que cabem matematicamente na altura ativa da tela ((tft.height() - inicioTextoY - 20) / 12). renderizarScroll computa adicionalmente proporções matemáticas para desenhar fisicamente uma barra de rolagem lateral dinâmica baseada no total do cache.

avancarLinha(): Move os eixos X e Y do cursor. Se o avanço ultrapassar as coordenadas finais do display, invoca o salto de renderização e recua o eixo Y visual, mantendo a digitação sempre visível.

Interface Gráfica e Visual (GUI) (os_gui.cpp)
Renderizador responsável pela divisão simétrica e pelos pixels da camada de apresentação.

desenharCabecalho(): Rotina de pintura estática. Calcula a largura total de caracteres em branco disponíveis, subtrai o comprimento do rótulo de título em andamento, e preenche as extremidades com caracteres de equalização (=) para garantir centralização matemática perfeita em qualquer dimensão de tela.

adicionarNoMenu(), atualizarListaMenu(): Alimentam o sub-motor do painel lateral. Geram os structs baseados nos objetos recebidos do explorador. Inspecionam as extensões das strings de nome via endsWith() e definem ponteiros numéricos fixos para as referências de ícones.

desenharMenu(), desenharIcone(): Iteram o vetor dinâmico de diretórios. Empregam lógica matemática de offset no eixo X e Y (baseados na profundidade da pasta) para gerar a identação da árvore visual. desenharIcone imprime gráficos de 8-bits pixel por pixel na coordenada calculada.

atualizarMarquee(): Motor de animação sem bloqueio baseado na função millis(). Compara o comprimento do nome do arquivo selecionado contra o limite espacial da interface. Se exceder, fatia a exibição (substring) avançando o marqueeOffset a cada 300ms, simulando letreiros de deslocamento horizontal contínuo.

desenharPopup(), fecharPopup(): Rotinas de sobreposição. Desenhadas para ignorar o buffer de trás e estampar a caixa de interrupção sobre tudo. fecharPopup força a FSM a restaurar a ramificação isolada do sistema e desencadeia a rotina de limpeza adequada dependendo se era o terminal ou o jogo ativo.

escreverEfeitoDigitacao(): Motor iterativo de delay. Analisa antecipadamente o tamanho da próxima palavra. Se a largura da letra somada ao cursor cruzar a coordenada máxima de leitura, o algoritmo adiciona um hífen visível no bloco atual e injeta forçadamente um avancarLinha() antes de concluir a palavra, evitando perda de texto em tela.

Aplicativos Integrados
Explorador de Arquivos (app_explorer.cpp)

acaoExplorador(): A ponte direta entre os dois chips. Para acessar os arquivos, ativa a variável uartOcupada = true (blindando a leitura contra as tarefas do Core 0). Dispara a instrução DADOS_SD e intercepta todos os pacotes em C-strings. Desmembra a arquitetura estruturada por | enviada pelo Pico com parsing hiperrápido via ponteiros strtok, constrói os nós encadeados em RAM e libera o processador.

limparCaracteresEspeciais(): Sanitizador de entrada. Inspeciona a tabela ASCII dos bytes enviados. Se um caractere apresentar mapeamento quebrado pela leitura física, ele substitui automaticamente o defeito por um _, preservando o laço de renderização principal de um desligamento por violação de acesso.

formatarTamanhoBytes(): Função de tradução que emprega lógica de divisões por 1024 em variáveis double de 64-bits (impedindo overflow), emitindo no terminal um tamanho humano perfeitamente escalonado em KB, MB ou GB.

Calculadora Científica (app_calc.cpp)

processarEntradaCalc(): Ambienta a interface gráfica em uma zona isolada. Ao receber strings com as equações matemáticas, despacha os dados textuais puros para compilação na biblioteca paralela (MathParser). Formata e exibe os cálculos na tela, assumindo controle absoluto das ações de interface (UP, DOWN, EXIT).

Terminal de Entretenimento (app_snake.cpp)

iniciarSnake(), sairSnake(): Inicializadores de subsistema. Alteram a trava de estado global, limpam matrizes bidimensionais do jogo e sobrepõem as interfaces visuais sem destruir o histórico do terminal de comandos no cache original.

atualizarPlacarSnake(), resetarFaseSnake(): Constroem a lógica geoespacial (X e Y) definindo as células, recriando o tamanho do jogador a cada nova rodada.

processarEntradaSnake(): Interpreta caracteres da Serial sob a restrição de que a cobra não pode inverter bruscamente seu sentido (ex: pressionar W enquanto anda para S).

atualizarJogoSnake(), desenharCobra(): O motor do jogo, que funciona em paralelo ao sistema operacional graças ao roteamento da FSM. Atualiza todas as coordenadas do corpo transferindo matrizes (a cabeça avança, o rabo é substituído por fundo preto). Dispara lógicas de verificação de invasão de memória (quando a cobra atinge seus próprios nós ou as bordas máximas de exibição calculadas pela matriz).
