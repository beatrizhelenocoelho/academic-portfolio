library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity MUX8x1 is
    Port (  g0 : in STD_LOGIC;
            g1 : in STD_LOGIC;
            g2 : in STD_LOGIC;
            d0 : in STD_LOGIC;
            d1 : in STD_LOGIC;
            d2 : in STD_LOGIC;
            d3 : in STD_LOGIC;
            d4 : in STD_LOGIC;
            d5 : in STD_LOGIC;
            d6 : in STD_LOGIC;
            d7 : in STD_LOGIC;
            y : out STD_LOGIC);
end MUX8x1;

architecture Behavioral of MUX8x1 is

signal sel : STD_LOGIC_VECTOR (2 downto 0);

begin
    -- concatenation of selection signals g2, g1 and g0
    -- to make a 3 bit input signal
    sel <= g2 & g1 & g0;

    y   <=  d0 when sel="000" else 
            d1 when sel="001" else 
            d2 when sel="010" else 
            d3 when sel="011" else 
            d4 when sel="100" else 
            d5 when sel="101" else 
            d6 when sel="110" else 
            d7;

end Behavioral;
