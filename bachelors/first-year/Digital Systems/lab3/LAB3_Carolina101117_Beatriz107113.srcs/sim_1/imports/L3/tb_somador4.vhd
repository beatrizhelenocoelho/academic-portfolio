----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 10/19/2017 01:47:13 PM
-- Design Name: 
-- Module Name: tb_somador4 - Behavioral
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
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity tb_somador4 is
--  Port ( );
end tb_somador4;

architecture Behavioral of tb_somador4 is
-- declaracao do componente
component somador4 
    Port ( P : in STD_LOGIC_VECTOR (3 downto 0);
           Q : in STD_LOGIC_VECTOR (3 downto 0);
           cin : in STD_LOGIC;
           S : out STD_LOGIC_VECTOR (3 downto 0);
           cout : out STD_LOGIC);
end component;
-- declaracao dos signais para o testbench
signal A, B : std_logic_vector (3 downto 0) := "0000";
signal ci : std_logic := '0';
signal S : std_logic_vector (3 downto 0);
signal co : std_logic;

begin
-- declaracao da instancia para teste
test_unit: somador4 port map ( 
                P => A, Q => B, cin => ci,
                S => S, cout => co);
                
-- gerador do sinal A (increments value every 25 ns)              
process 
begin
    wait for 25ns;
    A <= A + 1;
end process;

-- gerador do sinal B (increments value every 4*25 ns) 
-- it chages only when all combinations of A are exhausted (16 combinations)         
process 
begin
    wait for 16*25ns;
    B <= B + 1;
end process;

-- gerador do cin
-- keeps cin=0 for all cominations of A and B (16*16)
-- then changes to 1 for all combinations of A and B
process 
begin
    ci <= '0';
    wait for 16*16*25ns;
    ci <= '1';
    wait for 16*16*25ns;
end process;

end Behavioral;
