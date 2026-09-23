%% Close eerything and load data from files
close all
clear
clc

load("Exp_R_400_RE_0.001sqr_wave_30amp_02freq_KI0_02.mat")
ref = reference;
t_R_001 = y_plant.time;
y_R_001 = y_plant.signals.values;

load("Exp_R_400_RE_1000sqr_wave_30amp_02freq_KI0_02.mat")

t_R_1000 = y_plant.time;
y_R_1000 = y_plant.signals.values;

%%
figure;

plot(t_R_001, ref(1:length(t_R_001)), 'k--', t_R_001, y_R_001, 'b', t_R_1000, y_R_1000, 'r');
xlabel('Time (s)');
ylabel('Amplitude');
legend('Reference', 'R_E = 0.001', 'R_E = 1000', 'Location', 'best');
ylim([-40 40]);
grid on;


#Plot pre-filter reference vs plant output response

af_values = [0.3, 0.6, 0.9];

% Loop through each af value
for i = 1:length(af_values)
    af = af_values(i);
    
    filename = sprintf('Exp_R_400_af_%.1f', af);
    load(filename);
    
    figure;
    
    x = linspace(0, 401*0.02, 401);
    
    plot(x, reference.signals.values, 'r--', 'LineWidth', 1.2);
    hold on;
    plot(x, y_plant.signals.values, 'b', 'LineWidth', 1.2);
    
    xlabel('Time [s]');
    ylabel('Bar tip angular position [deg]');
    title(sprintf('Reference vs. Plant Output Response (af = %.1f)', af));
    legend('Reference', 'Plant output y_{plant}', 'Location', 'Best');
    grid on;
end

clear; clc; close all;

% Lista dos ficheiros (ordena como quiseres)
files = ["Exp_R_100.mat","Exp_R_200.mat","Exp_R_300.mat", ...
          "Exp_R_400.mat","Exp_R_500.mat","Exp_R_700.mat","Exp_R_1000.mat"];

% Extrair os valores de R automaticamente para as legendas
Rvals = [100 200 300 400 500 700 1000];

colors = lines(length(files)); % paleta de cores diferentes

%% --- Gráfico 1: Response of the system for different R values ---
figure('Color','w');
hold on; grid on;
for k = 1:length(files)
    load(files(k),"y_plant");  % carrega só essa variável
    plot(y_plant.time, y_plant.signals.values, ...
        'DisplayName', sprintf('R = %d', Rvals(k)), ...
        'Color', colors(k,:), 'LineWidth', 1.4);
end
xlabel('Time (s)');
ylabel('Response [deg]');
title('Response of the system for different R values');
legend('Location','bestoutside');
exportgraphics(gcf,'Response_different_R.png','Resolution',300);

%% --- Gráfico 2: Input of the system for different R values ---
figure('Color','w');
hold on; grid on;
for k = 1:length(files)
    load(files(k),"control_input");  % carrega só essa variável
    plot(control_input.time, control_input.signals.values, ...
        'DisplayName', sprintf('R = %d', Rvals(k)), ...
        'Color', colors(k,:), 'LineWidth', 1.4);
end
xlabel('Time (s)');
ylabel('Input / Actuation [V]');
title('Input of the system for different R values');
legend('Location','bestoutside');
exportgraphics(gcf,'Input_different_R.png','Resolution',300);


