----------------------------------------------------------------------------------
-- Company: IST
-- Course: Sistemas Digitais
-- 
-- Module Name: lab0 - Behavioral
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity lab0 is
    Port ( a1 : in STD_LOGIC;
           a2 : in STD_LOGIC;
           a3 : in STD_LOGIC;
           f : out STD_LOGIC);
end lab0;

architecture Behavioral of lab0 is

begin

f <= not(a2 and not(a2 and a1))and a3;

end Behavioral;
