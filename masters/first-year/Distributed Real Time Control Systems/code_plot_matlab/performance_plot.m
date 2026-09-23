clear;
clc;
close all;

% =========================================================
% BASE PERFORMANCE PLOT
% Coluna 1 -> time
% Coluna 2 -> ref
% Coluna 3 -> lux
% Coluna 4 -> pwm
% Coluna 5 -> u_percent
% Coluna 6 -> power
% Coluna 7 -> energy
% Coluna 8 -> avgVisibility
% Coluna 9 -> avgFlicker
% =========================================================

filename = 'performance_test.xlsx';   % mudar para o teu ficheiro
metricName = 'energy';  % escolher: 'energy', 'visibility', 'flicker'

data = readmatrix(filename);

time          = data(:,1);
refLux        = data(:,2);
measuredLux   = data(:,3);
u_percent     = data(:,5);
energy        = data(:,7);
avgVisibility = data(:,8);
avgFlicker    = data(:,9);

valid = all(~isnan(data), 2);
time          = time(valid);
refLux        = refLux(valid);
measuredLux   = measuredLux(valid);
u_percent     = u_percent(valid);
energy        = energy(valid);
avgVisibility = avgVisibility(valid);
avgFlicker    = avgFlicker(valid);

switch lower(metricName)
    case 'energy'
        metric = energy;
        metricLabel = 'Accumulated Energy (J)';
        figTitle = 'Illuminance and Accumulated Energy';
        legend3 = 'Accumulated Energy';
    case 'visibility'
        metric = avgVisibility;
        metricLabel = 'Average Visibility Error (lux)';
        figTitle = 'Illuminance and Visibility Error';
        legend3 = 'Average Visibility Error';
    case 'flicker'
        metric = avgFlicker;
        metricLabel = 'Average Flicker Error';
        figTitle = 'Illuminance and Flicker Error';
        legend3 = 'Average Flicker Error';
    otherwise
        error('metricName must be energy, visibility, or flicker');
end

figure;

subplot(2,1,1);

yyaxis left
plot(time, measuredLux, 'LineWidth', 1.5);
hold on;
plot(time, refLux, 'LineWidth', 1.5);
ylabel('Illuminance (lux)');
% exemplo:
% ylim([-1 18]);

yyaxis right
plot(time, metric, 'LineWidth', 1.5);
ylabel(metricLabel);

grid on;
xlabel('Time (s)');
title(figTitle);
legend('Measured Illuminance', 'Reference Illuminance', legend3, 'Location', 'best');

subplot(2,1,2);
plot(time, u_percent, 'LineWidth', 1.5);
grid on;
xlabel('Time (s)');
ylabel('u (%)');
title('Control Signal u(t)');
ylim([0 100]);