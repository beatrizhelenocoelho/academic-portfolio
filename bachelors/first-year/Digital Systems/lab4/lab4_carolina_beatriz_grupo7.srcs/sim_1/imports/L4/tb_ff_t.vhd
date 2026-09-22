----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 12/02/2016 02:39:40 PM
-- Design Name: 
-- Module Name: tb_ff_t - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity tb_ff_t is
--  Port ( );
end tb_ff_t;

architecture Behavioral of tb_ff_t is
signal T, CLK, SET, RESET, Q: std_logic := '0'; 

component ff_t
    Port (
	t 	: in STD_LOGIC;
        set   	: in STD_LOGIC;
        rst  	: in STD_LOGIC;
	clk 	: in STD_LOGIC;
	q 	: out STD_LOGIC);
end component;

begin

FF_T1: ff_t port map(
    t => T,
    clk => CLK,
    set => SET,
    rst => RESET,
    q => Q
);

process
    begin
    wait for 5 ns;
    CLK <= '0';
    wait for 10 ns;
    CLK <= '1';
    wait for 5 ns;
end process;

process
    begin
    T <= '0';
    wait for 50 ns;
    T <= '1';
    wait for 50 ns;
end process;

process
    begin
    SET <= '1';
    RESET <= '0';
    wait for 20 ns;
    SET <= '0';
    wait for 30 ns;
    RESET <= '1';
    wait for 20 ns;
    RESET <= '0';
    wait for 200 ns;
end process;



end Behavioral;
