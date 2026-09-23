clear;
clc;
close all;

% =========================================================
% BASE CONTROL PLOT
% Coluna 1 -> Reference lux
% Coluna 2 -> Measured lux
% Coluna 3 -> PWM
% Coluna 4 -> Time (s)
% =========================================================

filename = 'data.xlsx';   % nome do ficheiro

refLux = data(:,1);
measuredLux = data(:,2);
pwm = data(:,3);
time = data(:,4);

DAC_RANGE = 4095;
u_percent = (pwm ./ DAC_RANGE) * 100;

figure;

subplot(2,1,1);
plot(time, measuredLux, 'LineWidth', 1.5);
hold on;
plot(time, refLux, 'LineWidth', 1.5);
grid on;
xlabel('Time (s)');
ylabel('Illuminance (lux)');
title('Illuminance Response');
legend('Measured Illuminance', 'Reference Illuminance', 'Location', 'best');
% ylim([-5 25]);
% xlim([0 60]);

subplot(2,1,2);
plot(time, u_percent, 'LineWidth', 1.5);
grid on;
xlabel('Time (s)');
ylabel('u (%)');
title('Control Signal u(t)');
ylim([0 100]);
