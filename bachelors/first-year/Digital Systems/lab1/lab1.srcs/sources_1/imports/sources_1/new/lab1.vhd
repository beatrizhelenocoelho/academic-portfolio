----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 09/20/2018 06:01:52 AM
-- Design Name: 
-- Module Name: lab0 - Behavioral
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

entity lab1 is
    Port ( a3 : in STD_LOGIC;
           a2 : in STD_LOGIC;
           a1 : in STD_LOGIC;
           a0 : in STD_LOGIC;
           f : out STD_LOGIC);
end lab1;

architecture Behavioral of lab1 is

-- declaração de sinais auxliares para negação das entradas

signal na3, na2, na1, na0  : std_logic;


begin

-- Definição dos sinais auxiliares para negação das entradas

na3 <= not(a3);
na2 <= not(a2);
na1 <= not(a1);
na0 <= not(a0);
f <= a1 or na3 or na0 or na2;


end Behavioral;
