----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 09/23/2018 12:17:58 PM
-- Design Name: 
-- Module Name: tb_MUX4x1en - Behavioral
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

entity tb_MUX4x1 is
--  Port ( );
end tb_MUX4x1;

architecture Behavioral of tb_MUX4x1 is
-- decalaracao do componente
component MUX4x1
    Port (  g0 : in STD_LOGIC;
            g1 : in STD_LOGIC;
            d0 : in STD_LOGIC;
            d1 : in STD_LOGIC;
            d2 : in STD_LOGIC;
            d3 : in STD_LOGIC;
            y : out STD_LOGIC);
end component;
-- declaracao dos sinas para o testbench
signal g0, g1 : STD_LOGIC;
signal d0, d1, d2, d3 : STD_LOGIC;
signal y : STD_LOGIC;

begin
-- declaracao da unidade de teste
Utest: MUX4x1 port map (
            g0 => g0, g1 => g1, 
            d0 => d0, d1 => d1, d2 => d2, d3 => d3,
            y => y
        );
        
 -- descricao do gerador dos inputs d
process 
begin
    d0 <= '1';
    d1 <= '0';
    d2 <= '1';
    d3 <= '0';
    wait for 4*10 ns;
    d0 <= '1';
    d1 <= '1';
    d2 <= '1';
    d3 <= '1';
    wait for 4*10 ns;          
end process;
        
-- descricao do gerador dos sinal g0
process 
begin
    g0 <= '0';
    wait for 10 ns;
    g0 <= '1';
    wait for 10 ns;  
end process;

-- descricao do gerador dos sinal g1
process 
begin
    g1 <= '0';
    wait for 2*10 ns;
    g1 <= '1';
    wait for 2*10 ns;
end process;

end Behavioral;
