function plot_dados(tstId)
if nargin < 1
    tstId = 4;
end
fname = 'datalog_b_real_reset_12.DTX'; 
switch tstId
     % DATALOG A3
    case 1 
        titulo = 'Outputs';
        x = mem_dump_load(fname);
        
       % Extração dos dados
        
        % Separa os ciclos (posições ímpares: 1, 3, 5...) dos estados (posições pares: 2, 4, 6...)
        ciclos = x(1:2:end);
        dados_raw = x(2:2:end);
    
        % Conversão de Bits
        z_bits = dec2bin(dados_raw, 16) - '0'; 
        z_bits = z_bits(:, end:-1:1); % LSB (Bit 0) passa a ser a 1ª coluna
    
        % Garante que temos pelo menos 4 bits
        [num_amostras, num_bits] = size(z_bits);
        if num_bits < 4
            z_bits = [z_bits, zeros(num_amostras, 4 - num_bits)];
        end

        green_led  = z_bits(:, 1);  % OR 1 -> Bit 0
        yellow_led = z_bits(:, 2);  % OR 2 -> Bit 1
        red_led    = z_bits(:, 3);  % OR 4 -> Bit 2
        buzzer     = z_bits(:, 4);  % OR 8 -> Bit 3
    
        % Gráfico
        figure(202); clf;
        hold on; 

        stairs(ciclos, buzzer + 6,  'Color', [0, 0.443, 0.651], 'LineWidth', 2); 
        stairs(ciclos, red_led + 4,  'Color', [1, 0.145, 0.0], 'LineWidth', 2); 
        stairs(ciclos, green_led + 2, 'Color', [0.09, 0.71, 0.24], 'LineWidth', 2); 
        stairs(ciclos, yellow_led + 0, 'Color', [1,0.698,0.0] , 'LineWidth', 2); 
        
        % Formatação
        title(titulo);
        xlabel('Scan Cycle');
        
        % nomes das variáveis
        yticks([0, 2, 4, 6]);
        yticklabels({'Yellow LED', 'Green LED',  'Red LED', 'Buzzer'});
        
        ylim([-1 8]); 
        grid on;
        hold off;
        
    % DATALOG A4
    case 2
        titulo = 'Outputs';

        % Carregar os dados
        x = mem_dump_load(fname);

        % Extração dos dados

        % Separa os ciclos (posições ímpares: 1, 3, 5...) dos estados (posições pares: 2, 4, 6...)
        ciclos = x(1:2:end);
        dados_raw = x(2:2:end);

        idx_validos = find(ciclos > 0);
        if ~isempty(idx_validos)
            ciclos = ciclos(idx_validos);
            dados_raw = dados_raw(idx_validos);
        end

        % Conversão de Bits
        z_bits = dec2bin(dados_raw, 16) - '0'; 
        z_bits = z_bits(:, end:-1:1); % LSB (Bit 0) passa a ser a 1ª coluna

       % Garante que temos pelo menos 5 bits
       [num_amostras, num_bits] = size(z_bits);
        if num_bits < 5
            z_bits = [z_bits, zeros(num_amostras, 4 - num_bits)];
        end

     
        Mode1              = z_bits(:, 1);  % OR 1 -> Bit 0
        Mode2              = z_bits(:, 2);  % OR 2 -> Bit 1
        AlternativeMode    = z_bits(:, 3);  % OR 4 -> Bit 2
        Off                = z_bits(:, 4);  % OR 8 -> Bit 3
        buzzer             = z_bits(:, 5);  % OR 8 -> Bit 3

        % Gráfico
        figure(202); clf;
        hold on; 

        stairs(ciclos, Mode1 + 8, 'Color', [0.388, 0.6, 0.0], 'LineWidth', 2); 
        stairs(ciclos, Mode2 + 6, 'Color', [0.53, 0.83, 0.0], 'LineWidth', 2); 
        stairs(ciclos, AlternativeMode + 4,  'Color', [1, 0.608, 0.0], 'LineWidth', 2); 
        stairs(ciclos, Off + 2,  'Color', [1, 0.145, 0.0], 'LineWidth', 2); 
        stairs(ciclos, buzzer + 0,  'Color', [0, 0.443, 0.651], 'LineWidth', 2); 

        % Formatação
        title(titulo);
        xlabel('Scan Cycle');

        % nomes das variáveis
        yticks([0, 2, 4, 6, 8]);
        yticklabels({'Buzzer', 'Mode OFF', 'Mode Alternating', 'Mode 2 - ON', 'Mode 1 - ON'});

        ylim([-1.5 10.5]);
        xlim([0 max(ciclos)]);
        grid on;
        hold off;
    
   % DATALOG A6
    case 3

        titulo = 'Outputs';
        x = mem_dump_load(fname);

        ciclos = x(1:2:end);
        dados_raw = x(2:2:end);

        idx_validos = find(ciclos > 0);
        if ~isempty(idx_validos)
            ciclos = ciclos(idx_validos);
            dados_raw = dados_raw(idx_validos);
        end

        % Conversão de Bits
        z_bits = dec2bin(dados_raw, 16) - '0'; 
        z_bits = z_bits(:, end:-1:1); % LSB (Bit 0) passa a ser a 1ª coluna
        
        M_MODE = z_bits(:, 1) + z_bits(:, 2)*2;

        I_presence   = z_bits(:, 3);  % OR 1 -> Bit 0
        I_window     = z_bits(:, 4);  % OR 2 -> Bit 1
        I_key        = z_bits(:, 5);  % OR 4 -> Bit 2
        green_led    = z_bits(:, 6);  % OR 1 -> Bit 0
        yellow_led   = z_bits(:, 7);  % OR 2 -> Bit 1
        red_led      = z_bits(:, 8);  % OR 4 -> Bit 2
        buzzer       = z_bits(:, 9);  % OR 8 -> Bit 3
        

        % Gráfico
        figure(202); clf;
        hold on; 

        stairs(ciclos, M_MODE + 13, 'Color' ,[0.529, 0.525, 0.451], 'LineWidth', 2);
        stairs(ciclos, I_key + 12,  'Color' ,[0.561, 0, 0.263], 'LineWidth', 2);
        stairs(ciclos, I_window + 10,  'Color' ,[0, 0.557, 1], 'LineWidth', 2); 
        stairs(ciclos, I_presence + 8, 'Color', [1, 0.443, 0], 'LineWidth', 2);
        stairs(ciclos, buzzer + 6,  'Color', [0, 0.443, 0.651], 'LineWidth', 2); 
        stairs(ciclos, green_led + 4, 'Color', [0.09, 0.71, 0.24], 'LineWidth', 2); 
        stairs(ciclos, yellow_led + 2, 'Color', [1,0.698,0.0] , 'LineWidth', 2); 
        stairs(ciclos, red_led + 0,  'Color', [1, 0.145, 0.0], 'LineWidth', 2); 


        
        

        % Formatação
        title(titulo);
        xlabel('Scan Cycle');

         % nomes das variáveis
        yticks([0, 2, 4, 6, 8, 10, 12, 14, 15, 16]);
        yticklabels({'Red LED', 'Yellow LED', 'Green LED', 'Buzzer', 'Infrared Sensor', 'Window Switch', '# Key', '1', 'M\_MODE    2', '3'});

        ylim([-1 17]);
        xlim([0 max(ciclos)]);
        grid on;
        hold off;

   % Exercicio B
    case 4
        
        % Extração dos dados
        x = mem_dump_load(fname);
        
        % Garantir que x tem número par de elementos
        if mod(numel(x), 2) ~= 0
            x = x(1:end-1);  % descarta o último elemento ímpar
        end
        
        ciclos    = x(1:2:end);
        dados_raw = x(2:2:end);
        
        % Filtrar apenas ciclos válidos (> 0)
        idx_validos = find(ciclos > 0);
        if ~isempty(idx_validos)
            ciclos    = ciclos(idx_validos);
            dados_raw = dados_raw(idx_validos);  % agora idx_validos nunca excede dados_raw
        end
        
        % Conversão de Bits
        z_bits = dec2bin(dados_raw, 16) - '0';
        z_bits = z_bits(:, end:-1:1); % LSB primeiro
        
        % Bits 0–7: alarmes
        line_a  = z_bits(:, 1);  % bit 0
        line_b  = z_bits(:, 2);  % bit 1
        line_c  = z_bits(:, 3);  % bit 2
        line_d  = z_bits(:, 4);  % bit 3
        col_1   = z_bits(:, 5);  % bit 4
        col_2   = z_bits(:, 6);  % bit 5
        col_3   = z_bits(:, 7);  % bit 6
        buzzer = z_bits(:, 8);

        % Bits 8–11: current_key (0–15)
        current_key = bitshift(bitand(dados_raw, 3840), -8);

        % Bits 12–14: msg_index (0–7)
        msg_index = bitshift(bitand(dados_raw, 61440), -12);

        % Gráfico
        figure(202); clf;
        
        % 1º Gráfico: Linhas e Colunas (Topo)
        ax1 = subplot(3,1,1); % 3 Linhas, 1 Coluna, Posição 1
        hold on;
        stairs(ciclos, line_a  + 24, 'g', 'LineWidth', 2);
        stairs(ciclos, line_b  + 22, 'g', 'LineWidth', 2);
        stairs(ciclos, line_c  + 20, 'g', 'LineWidth', 2);
        stairs(ciclos, line_d  + 18, 'g', 'LineWidth', 2);
        stairs(ciclos, col_1   + 16, 'b', 'LineWidth', 2);
        stairs(ciclos, col_2   + 14, 'b', 'LineWidth', 2);
        stairs(ciclos, col_3   + 12, 'b', 'LineWidth', 2);
        stairs(ciclos, buzzer   + 10, 'r', 'LineWidth', 2);
        yticks([10.5, 12.5, 14.5, 16.5, 18.5, 20.5, 22.5, 24.5]);
        yticklabels({'buzzer','col\_3','col\_2','col\_1','line\_d','line\_c','line\_b','line\_a'});
        ylim([9 26]);
        xlim([ciclos(1), ciclos(end)]);
        grid on; hold off;
        
        % 2º Gráfico: Current Key (Meio)
        ax2 = subplot(3,1,2); % 3 Linhas, 1 Coluna, Posição 2
        stairs(ciclos, current_key, 'k', 'LineWidth', 2);
        yticks(0:12);
        ylabel('current\_key');
        xlim([ciclos(1), ciclos(end)]);
        grid on; 
        
        % 3º Gráfico: Buffer Index (Fundo)
        ax3 = subplot(3,1,3); % 3 Linhas, 1 Coluna, Posição 3
        stairs(ciclos, msg_index, 'k', 'LineWidth', 2);
        yticks(0:11);
        ylabel('buffer\_index');
        xlabel('Número do Ciclo (Scan Cycle)'); 
        xlim([ciclos(1), ciclos(end)]);
        grid on; 
        
        % Liga o eixo X de todos os gráficos! 
        linkaxes([ax1, ax2, ax3], 'x');
        
otherwise
        error('tstId inválido')
end
return