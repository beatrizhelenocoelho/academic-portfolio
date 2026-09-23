%% Load matrices
close all
clear
clc
load("SS_4_Model.mat");
A = sys.A; B = sys.B; C = sys.C; D = sys.D; Ts = 0.02;
G = ss(A, B, C, 0, Ts);
%%
Kp = -34.5846; Ke = 1.9213;
K_cal = [Kp Ke 0 0];
%% Compute the Nu and Nx values to allow for the introduction of a reference
N = inv([A-eye(size(A)), B; C,0])*[zeros(size(A,1),1);1];
Nx = N(1:end-1,:);
Nu = N(end,:);

%% Calculate LQR vector gain and compensation gain
Q = C'*C;
R = 400;
[K, ~, E] = dlqr(A, B, Q, R);
Nbar = Nu + K * Nx;

%% Symmetric Root Locus
[NG,DG] = tfdata(G,'v');
[NG,DG] = eqtflength(NG,DG);
SRL = tf(conv(NG,fliplr(NG)),conv(DG,fliplr(DG)));
p_srl = rlocus(SRL,1/R);
subplot(1,2,2)
zplane([],p_srl)
title(['SRL poles for \rho = 1/R = ' num2str(1/R)])
ax = axis;
subplot(1,2,1)
rlocus(SRL)
hold on; zplane([],[]); hold off
axis(2*ax)

%% Compute LQR and LQG open-loop transfer functions, for different weights in LQG
T_lqr = ss(A,B,K,0,Ts);

possibilities_w = [0.1 1 10 500];
figure;
hold on;
bode(T_lqr);
title("Bode Diagram for different values of QE")
legendEntries = {"LQR"};

i =1;
for w2 = possibilities_w
    QE = eye(size(A))*w2;
    RE = 1;
    G1 = eye(size(A));
    [M,P,Z,EE] = dlqe(A,G1,C,QE,RE);
    aux1{i} = EE;
    PHIE = A-M*C*A;
    GAMMAE = B-M*C*B;

    T2_lqg = ss([A zeros(size(A)); M*C*A PHIE-GAMMAE*K], ...
     [B; M*C*B],[zeros(size(K)) K],0,Ts);
    opts = bodeoptions('cstprefs');
    opts.PhaseWrapping = 'on';
    h = bodeplot(T2_lqg, opts);
    hold on;
    set(findall(gcf,'Type','line'),'LineWidth',1.1);
    legendEntries{end+1} = sprintf('LQG (w = %g, R_E = %g)', w2, RE);
    h.showCharacteristic('AllStabilityMargins')
    i = i+1;
end

legend(legendEntries, 'Location', 'best');
grid on;

%% Compute LQE vector gain for the chosen weight matrices
w2 = 100; QE = eye(size(A))*w2;
RE = 1;
G1 = eye(size(A));
[M,P,Z,EE] = dlqe(A,G1,C,QE,RE);
PHIE = A-M*C*A;
GAMMAE = B-M*C*B;

%% Design of the pre-filter
af = 0.9;
Afilt = [1 -af];
Bfilt = 1-af;

%% Compute the LQG closed-loop and plot the bode diagram
C2_lqg = ss([A -B*K; M*C*A PHIE-GAMMAE*K-M*C*B*K], ...
 [B; M*C*B+GAMMAE]*Nbar,[C zeros(size(C))],0,Ts);

figure;
opts = bodeoptions('cstprefs');
opts.PhaseWrapping = 'off';
bodeplot(C2_lqg, opts);
set(findall(gcf,'Type','line'),'LineWidth',1.1);
legend('LQG');
grid on;