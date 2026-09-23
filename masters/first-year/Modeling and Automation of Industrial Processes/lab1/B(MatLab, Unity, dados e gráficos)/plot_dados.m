function plot_dados(tstId)

if nargin < 1
    tstId = 1;
end

switch tstId
    case 1
        fname = 'B2/DataB2/data_2_760.DTX'; 
        % Titulo B1
        %titulo = 'Atraso no circuito RC (sem potênciometro)';
        % Tilulo B2
        titulo = 'Atraso no circuito RC (com potênciometro)';
        
        x = mem_dump_load(fname);
        idx_final = find(diff(x(1,:)) > 0, 1, 'last') + 1;
        
        if ~isempty(idx_final)
            x = x(:, 1:idx_final);
        end
    
        % Conversão de Bits
        z_bits = dec2bin(x(2,:), 16) - '0'; 
        z_bits = z_bits(:, end:-1:1);
        
        tempo = x(1,:)/1000;
    
        saida = z_bits(:, 1);  % Bit 0
        entrada = z_bits(:, 2); % Bit 1
    
        % Gráfico
        figure(202); clf;
        hold on; 
    
        % Desenha a Saída (Azul)
        stairs(tempo, saida, 'b', 'LineWidth', 4); 
    
        % Desenha a Entrada (Laranja)
        stairs(tempo, entrada, 'r', 'LineWidth', 2); 
        
        % Formatação
        title(titulo);
        xlabel('Tempo (Segundos)');
        ylabel('Estado Lógico (0=OFF, 1=ON)');
        ylim([-0.1 1.2]);
        legend('Output', 'Input');
        grid on;
        hold off;

    otherwise
            error('inv tstId')
    end

return