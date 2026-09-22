----------------------------------------------------------------------------------
--
-- Description:   flip-flop tipo T com set e reset síncrono
-- 
----------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity ff_t is
    Port ( 
	t 	: in STD_LOGIC;
	set   	: in STD_LOGIC;
	rst  	: in STD_LOGIC;
	clk 	: in STD_LOGIC;
	q   	: out STD_LOGIC);
end ff_t;

architecture Behavioral of ff_t is
signal q_buffer : STD_LOGIC := '0'; 
signal q_next : STD_LOGIC;
begin
    	
q_next <= '0' when rst='1' else 
		   '1' when set='1' else
		   not q_buffer when t='1' else
		   q_buffer;
        
q_buffer <= q_next when rising_edge(clk);

q <= q_buffer;
	  
end Behavioral;
