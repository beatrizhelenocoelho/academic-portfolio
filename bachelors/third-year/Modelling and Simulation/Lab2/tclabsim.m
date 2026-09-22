function y= tclabsim(t,x0,u,p)


Q = zeros(525, 2);
Q(:, 1) = t;
Q(:, 2) = u(1, :); % Atribuir o valor da primeira linha de u à primeira linha de Q


U=p(1);
alpha =p(2);
tau =p(3);

x0 = x0 +273.15;

simulacao = sim('circuito_base_final.slx', 'SrcWorkspace','current');

y = simulacao.TS_sim(1:length(t)).'; % Corrigido para 1x525
y = y(:)'; % Forçar o formato 1x525 (linha)

end