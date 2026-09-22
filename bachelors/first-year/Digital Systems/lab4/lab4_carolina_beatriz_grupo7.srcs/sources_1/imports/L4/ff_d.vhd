----------------------------------------------------------------------------------
--
-- Description:   flip-flop tipo D com set e reset síncrono
-- 
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity ff_d is
    Port (
	d 	: in STD_LOGIC;
        set   	: in STD_LOGIC;
        rst  	: in STD_LOGIC;
	clk 	: in STD_LOGIC;
	q 	: out STD_LOGIC);
end ff_d;

architecture Behavioral of ff_d is

signal q_buffer : STD_LOGIC := '0';
signal q_next : STD_LOGIC;

begin

    q_next <= '0' when rst='1' else 
		        '1' when set='1' else
		        d;

    q_buffer <= q_next  when rising_edge(clk);

    q <= q_buffer;
end Behavioral;
