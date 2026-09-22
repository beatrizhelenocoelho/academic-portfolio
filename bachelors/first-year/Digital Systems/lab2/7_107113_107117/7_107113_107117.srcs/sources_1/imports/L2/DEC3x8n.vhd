library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity DEC3x8n is
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
end DEC3x8n;

architecture Behavioral of DEC3x8n is

signal d : STD_LOGIC_VECTOR (2 downto 0);
signal y : STD_LOGIC_VECTOR (7 downto 0);

begin
    -- concatenation of input signals d4, d2 and d1
    -- to make a 3 bit input signal
    d <= d4 & d2 & d1;
    -- internal 8-bit output
    y <=    "11111111" when en='0' else
            "11111110" when d="000" else
            "11111101" when d="001" else
            "11111011" when d="010" else
            "11110111" when d="011" else
            "11101111" when d="100" else
            "11011111" when d="101" else
            "10111111" when d="110" else
            "01111111";
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
