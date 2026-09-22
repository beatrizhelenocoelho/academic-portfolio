----------------------------------------------------------------------------------
--
-- Description:   Circuito sequencial básico
--                           NAO MODIFICAR
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;


entity sequencial is
	Port (ini : in STD_LOGIC;
		clk : in STD_LOGIC;
		m : in STD_LOGIC;
		B : in STD_LOGIC_VECTOR (2 downto 0);
		Q : out STD_LOGIC_VECTOR (2 downto 0));
end sequencial;

architecture Behavioral of sequencial is

signal d_2, d_1, t_0: STD_LOGIC;
signal SET, RESET, Qbuf: STD_LOGIC_VECTOR (2 downto 0);

component ff_d is
    Port ( 
	d 	: in STD_LOGIC;
	set   	: in STD_LOGIC;
	rst  	: in STD_LOGIC;
	clk 	: in STD_LOGIC;
	q   	: out STD_LOGIC);
end component;

component ff_t is
    Port ( 
	t 	: in STD_LOGIC;
	set   	: in STD_LOGIC;
	rst  	: in STD_LOGIC;
	clk 	: in STD_LOGIC;
	q   	: out STD_LOGIC);
end component;


begin

FFD2 : ff_d port map (
    d => d_2,
    set => SET(2), 
    rst => RESET(2), 
    clk => clk,
    q => Qbuf(2)  
); 

FFD1 : ff_d port map (
    d   => d_1,
    set => SET(1),
    rst => RESET(1), 
    clk => clk,
    q => Qbuf(1)  
); 

FFT0 : ff_t port map (
    t => t_0,
    set => SET(0),
    rst => RESET(0), 
    clk => clk,
    q => Qbuf(0)  
); 
-- sets and resets
SET <= (ini & ini & ini) and B;
RESET <= (ini & ini & ini) and not B;


d_2 <= (Qbuf(2) and not(Qbuf(1)) and not m) or 
        (Qbuf(2) and Qbuf(1) and m) or 
        (not(Qbuf(2)) and Qbuf(1) and not m) or
        (not(Qbuf(2)) and not(Qbuf(0)) and m);

d_1 <= (Qbuf(2) and not Qbuf(1)) 
        or (not Qbuf(1) and not m)   
	or (not Qbuf(2) and not Qbuf(0) and m); 
	 
t_0 <= (Qbuf(2) and Qbuf(1) and not m) 
	or (not Qbuf(2) and Qbuf(1) and not Qbuf(0) and m) 
	or (not Qbuf(2) and not Qbuf(1) and Qbuf(0) and m);

	
Q <= Qbuf;

end Behavioral;
