library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity DEC3x8 is
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
end DEC3x8;

architecture Behavioral of DEC3x8 is

signal d : STD_LOGIC_VECTOR (2 downto 0);
signal y : STD_LOGIC_VECTOR (7 downto 0);

begin
    -- concatenation of input signals d4, d2 and d1
    -- to make a 3 bit input signal
    d <= d4 & d2 & d1;
    -- internal 8-bit output
    y <=    "00000000" when en='0' else
            "00000001" when d="000" else
            "00000010" when d="001" else
            "00000100" when d="010" else
            "00001000" when d="011" else
            "00010000" when d="100" else
            "00100000" when d="101" else
            "01000000" when d="110" else
            "10000000";
    -- atribuition of internal bits to ouput signals
    y0 <= y(0);
    y1 <= y(1);
    y2 <= y(2);
    y3 <= y(3);
    y4 <= y(4);
    y5 <= y(5);
    y6 <= y(6);
    y7 <= y(7);

end Behavioral;
