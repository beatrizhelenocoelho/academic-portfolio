----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 10/18/2017 10:23:32 AM
-- Design Name: 
-- Module Name: tb_ff_d - Behavioral
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

entity tb_lab1 is
--  Port ( );
end tb_lab1;

architecture Behavioral of tb_lab1 is
-- declaracao do componente
component lab1 
    Port ( a3 : in STD_LOGIC;
	       a2 : in STD_LOGIC;
           a1 : in STD_LOGIC;
           a0 : in STD_LOGIC;
           f : out STD_LOGIC);
end component;
-- declaracao dos signais para o testbench
signal a3, a2, a1, a0, f : std_logic;

begin
-- declaracao da instancia para teste
test_unit: lab1 port map ( 
                a3 => a3, a2 => a2, 
                a1 => a1, a0 => a0, 
                f => f);
                
	process begin
	  a0 <= '0';
	  wait for 25 ns;
	  a0 <= '1';
	  wait for 25 ns;
	end process;
	
	process begin
	  a1 <= '0';
	  wait for 50 ns;
	  a1 <= '1';
	  wait for 50 ns;
	end process;

	process begin
	  a2 <= '0';
	  wait for 100 ns;
	  a2 <= '1';
	  wait for 100 ns;
	end process;
	
	process begin
	  a3 <= '0';
	  wait for 200 ns;
	  a3 <= '1';
	  wait for 200 ns;
	end process;


end Behavioral;
