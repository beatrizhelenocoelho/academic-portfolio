----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 09/23/2018 12:17:58 PM
-- Design Name: 
-- Module Name: tb_DEC3x8n - Behavioral
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

entity tb_DEC3x8n is
--  Port ( );
end tb_DEC3x8n;

architecture Behavioral of tb_DEC3x8n is
-- decalaracao do componente
component DEC3x8n
    Port ( en : in STD_LOGIC;
       d1 : in STD_LOGIC;
       d2 : in STD_LOGIC;
       d4 : in STD_LOGIC;
       y0 : out STD_LOGIC;
       y1 : out STD_LOGIC;
       y2 : out STD_LOGIC;
       y3 : out STD_LOGIC;
       y4 : out STD_LOGIC;
       y5 : out STD_LOGIC;
       y6 : out STD_LOGIC;
       y7 : out STD_LOGIC);
end component;
-- desclaracao dos sinas para o testbench
signal en : STD_LOGIC;
signal d1, d2, d4 : STD_LOGIC;
signal y0, y1, y2, y3, y4, y5, y6, y7 : STD_LOGIC; 

begin

-- declaracao da unidade de teste
Utest: DEC3x8n port map (
            en => en, 
            d1 => d1, d2 => d2, d4 => d4,
            y0 => y0, y1 => y1, y2 => y2, y3 => y3,
            y4 => y4, y5 => y5, y6 => y6, y7 => y7
        );

-- descricao do gerador dos sinal EN
process 
begin
    en <= '1';
    wait for 8*10 ns;
    en <= '0';
    wait for 8*10 ns;          
end process;

-- descricao do gerador dos sinal d1
process 
begin
    d1 <= '0';
    wait for 10 ns;
    d1 <= '1';
    wait for 10 ns;
       
end process;

-- descricao do gerador dos sinal d2
process 
begin
    d2 <= '0';
    wait for 2*10 ns;
    d2 <= '1';
    wait for 2*10 ns;
end process;

-- descricao do gerador dos sinal d4
process 
begin
    d4 <= '0';
    wait for 4*10 ns;
    d4 <= '1';
    wait for 4*10 ns;
end process;

end Behavioral;
