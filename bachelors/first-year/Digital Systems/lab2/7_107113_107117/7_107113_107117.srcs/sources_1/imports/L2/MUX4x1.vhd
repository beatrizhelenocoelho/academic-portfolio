library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity MUX4x1 is
    Port (  g0 : in STD_LOGIC;
            g1 : in STD_LOGIC;
            d0 : in STD_LOGIC;
            d1 : in STD_LOGIC;
            d2 : in STD_LOGIC;
            d3 : in STD_LOGIC;
            y : out STD_LOGIC);
end MUX4x1;

architecture Behavioral of MUX4x1 is

signal sel : STD_LOGIC_VECTOR (1 downto 0);

begin
    -- concatenation of selection signals g1 and g0
    -- to make a 2 bit input signal
    sel <= g1 & g0;

    y   <=  d0 when sel="00" else 
            d1 when sel="01" else 
            d2 when sel="10" else 
            d3;

end Behavioral;
