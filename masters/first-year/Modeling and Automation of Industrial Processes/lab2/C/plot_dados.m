% =========================================================
% GRAFICO 2 - RESET MANUAL COM TECLAS
%
% Mostra:
% - teclas antes/depois
% - msg_index antes/depois do reset
% - I_alarm / reset_buffer
% =========================================================

idx_reset = find(I_alarm == 1 | reset_buffer == 1, 1, 'first');

if isempty(idx_reset)
    warning('Nao encontrei I_alarm/reset_buffer. Vou mostrar o inicio do ensaio.');
    ini_reset = 1;
    fim_reset = min(length(amostra), 30);
else
    ini_reset = max(1, idx_reset - 8);
    fim_reset = min(length(amostra), idx_reset + 10);
end

x_reset = amostra(ini_reset:fim_reset);
r_reset = ini_reset:fim_reset;

fig_reset = figure('Name','Parte C - Reset manual com teclas','Color','w');
set(fig_reset, 'Position', [150 150 900 540]);

subplot(3,1,1)
stairs(x_reset, current_key(r_reset), 'LineWidth', 2)
grid on
ylim([-1.5 12])
yticks([-1 0 1 2 3 4 5 6 7 8 9 10 11])
yticklabels({'sem','0','1','2','3','4','5','6','7','8','9','*','#'})
ylabel('Tecla')
title('Reset manual: teclas introduzidas antes/depois')

subplot(3,1,2)
stairs(x_reset, msg_index(r_reset), 'LineWidth', 2)
grid on
ylim([-0.5 6])
yticks(0:6)
ylabel('msg\_index')
title('Limpeza da sequência introduzida')

subplot(3,1,3)
stairs(x_reset, I_alarm(r_reset), 'LineWidth', 2)
hold on
stairs(x_reset, reset_buffer(r_reset), '--', 'LineWidth', 2)
grid on
ylim([-0.1 1.1])
yticks([0 1])
ylabel('Sinal')
xlabel('Amostra')
legend('I\_alarm','reset\_buffer','Location','best')
title('Acionamento do reset manual')

print(fig_reset, 'parteC_reset_manual_COM_TECLAS.png', '-dpng', '-r200');