----------------------------------------------------------------------------------
--
-- Description:   Contador de 4 bits, baseado no circuito sequencial básico (saida Q do sequencial2)  
--                e num elemento de memória adicional que deve contar
--                5->D->0->E->0->F->1->C->4->E  
-- 
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity Lab4 is
    Port ( 
	ini : in STD_LOGIC;
	clk : in STD_LOGIC;
	Y : out STD_LOGIC_VECTOR (3 downto 0);
	S : out STD_LOGIC_VECTOR (3 downto 0);
	Q : out STD_LOGIC_VECTOR (2 downto 0);
	t : out STD_LOGIC);
	   
end Lab4;

architecture Behavioral of lab4 is
-- declaracao dos componentes SEQUENCIAL2 e FF_T
component sequencial2 
    Port ( 
	   ini : in STD_LOGIC;
	   clk : in STD_LOGIC;
        S : out STD_LOGIC_VECTOR (3 downto 0);
        Q : out STD_LOGIC_VECTOR (2 downto 0));
end component;

component ff_t is
    Port ( 
	t 	: in STD_LOGIC;
	set   	: in STD_LOGIC;
	rst  	: in STD_LOGIC;
	clk 	: in STD_LOGIC;
	q   	: out STD_LOGIC);
end component;


component decoder is
    Port ( 
	I 	: in STD_LOGIC_VECTOR (2 downto 0);
	en  : in STD_LOGIC;
	O   : out STD_LOGIC_VECTOR (7 downto 0));
end component;

component decoder_neg is
    Port ( 
	I 	: in STD_LOGIC_VECTOR (2 downto 0);
    en  : in STD_LOGIC;
    O   : out STD_LOGIC_VECTOR (7 downto 0));
end component;


-- declaracao dos sinais internos 
signal seq2_S : std_logic_vector (3 downto 0);
signal  seq_Q : std_logic_vector (2 downto 0);

signal  dec_O : std_logic_vector (7 downto 0);
signal  dec_neg_O : std_logic_vector (7 downto 0);

signal seq2_ini, fft_ini, fft_t,  fft_q, not_fft_q: std_logic;
-- declaracao do sinais adicionais que possam ser necessários



begin
-- instância do SEQUENCIAL2 (port map)
-- não modificar
SEQ2: sequencial2 port map (
        ini => seq2_ini, 
        clk => clk,
	    S => seq2_S,
	    Q => seq_Q 
    );
    
-- instancia do FF_T (port map)
-- não modificar
FFT : ff_t port map (
    t => fft_t,
    clk => clk,
    set => '0',
    rst => fft_ini, 
    q   => fft_q  
); 
not_fft_q <= not(fft_q );

-- instancia do DEC_NEG (port map)
-- não modificar
DEC : decoder port map (
    en => fft_q,
    I => seq_Q,
    O   => dec_O
); 
-- instancia do DEC_NEG (port map)
-- não modificar
DEC_NEG : decoder_neg port map (
    en => not_fft_q,
    I => seq_Q,
    O   => dec_neg_O
); 
    
-- atribuição das entradas do circuito SEQ2 e do FFT
-- DEVE SER MODIFICADO para uma funcao de dec_neg_O, dec_O e INI  
  seq2_ini <= dec_O(3) or ini;      
  fft_ini  <= dec_O(3) or ini;         ---dar reset que vai parar no 3---
  fft_t    <= not(dec_neg_O (6));      --- para controlar quando usamos a parte de cima ou de baixo da tablea  entrada t quamdo chega ao 6 queremos que comece a dar reset,
                                       ---- temos que usrar o negado pois o T (toggle começa a 0 - muda a função quando o valor é 1

-- atribuição das saída do circuito lab 3
-- DEVE SER MODIFICADO  para uma funcao de dec_neg_O, dec_O
  Y(0) <= not (dec_neg_O(3) and dec_neg_O(4))or dec_O (1);
  Y(1) <= not (dec_neg_O(0) and dec_neg_O(1)and dec_neg_O(2)and dec_neg_O(3))  or  dec_O(1);
  Y(2) <=  not (dec_neg_O(1) and dec_neg_O(2) and dec_neg_O(3)and dec_neg_O(6)and dec_neg_O(7))  or dec_O (3) ;
  Y(3) <= not (dec_neg_O(0) and dec_neg_O(2) and dec_neg_O(3) and dec_neg_O(6) and dec_neg_O(7)) or dec_O(1);

   
-- passagem das saída do circuito sequenciais e FFT para visualização na placa
-- não modificar
    Q <= seq_Q;
    S <= seq2_S;
    t <= fft_q;      
    
end Behavioral;
