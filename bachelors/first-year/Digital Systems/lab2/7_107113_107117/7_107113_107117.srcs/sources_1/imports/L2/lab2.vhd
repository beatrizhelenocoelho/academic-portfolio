--------------------------------------------------------------
--  <7>
-- <Carolina Baltazar> (<107117>)
-- <Beatriz Coelho> (<107113>)
--------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity lab2 is
    Port ( A : in STD_LOGIC_VECTOR (3 downto 0);
           P : in STD_LOGIC_VECTOR (1 downto 0);
           C : out STD_LOGIC_VECTOR (3 downto 0);
           S : out STD_LOGIC);
end lab2;

architecture Behavioral of lab2 is
-- declaration of components (DEC, MUX...)

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

component DEC3x8 
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

component MUX8x1 
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
end component;

component MUX4x1 
    Port (  g0 : in STD_LOGIC;
            g1 : in STD_LOGIC;
            d0 : in STD_LOGIC;
            d1 : in STD_LOGIC;
            d2 : in STD_LOGIC;
            d3 : in STD_LOGIC;
            y : out STD_LOGIC);
end component;

-- declaration of internal signals (fios)
signal c0, c1,c2,c3: std_logic;
signal y0, y1, y2, y3, y4,y5,y6,y7, y :std_logic;
signal x0, x1,x2, x3, x4,x5,x6,x7 :std_logic;
signal w0, w1, w2, w3, w4,w5,w6,w7 :std_logic;


-- o sinal interno cat vai identificar a categoria corresponde ao codigo do produto (A3,A2,A1,A0)
signal cat : std_logic_vector(3 downto 0);
signal na3, na0 : std_logic;
-- inclua abaixo todos os outros sinais internos necessarios


begin
na3 <= not(A(3));
na0 <= not(A(0));
-- definition of internal signals 
c0 <= y;
c1 <= not (x4 and x5);
c2 <=not (y0);
c3 <= (w4 or w7);  
 
-- and instances of components (port map)
b1: DEC3x8n port map(
    en=> A(3),
    d1=>A(2),
    d2=>A(1),
    d4=>A(0),
    y0=>x0,
    y1=>x1,
    y2=>x2,
    y3=>x3,
    y4=>x4,
    y5=>x5,
    y6=>x6,
    y7=>x7
   );
   


b2: DEC3x8n port map(
    en=> A(1),
    d1=>A(3),
    d2=>A(2),
    d4=>A(0),
    y0=>y0,
    y1=>y1,
    y2=>y2,
    y3=>y3,
    y4=>y4,
    y5=>y5,
    y6=>y6,
    y7=>y7);



b3: DEC3x8 port map(
    en=> na3,
    d1=>A(2),
    d2=>A(1),
    d4=>A(0),
    y0=>w0,
    y1=>w1,
    y2=>w2,
    y3=>w3,
    y4=>w4,
    y5=>w5,
    y6=>w6,
    y7=>w7
    );
b4: MUX8x1 port map(
   
    g0=> A(3),
    g1=> A(2),
    g2=> A(1),
    y=>c0,
    d0=>na0,
    d1 => A(0),
    d2 => A(0),
    d3=>na0,
    d4 => A(0),
    d5=> na0,
    d6 => A(0),
    d7 => A(0)
    );
b5: MUX4x1 port map(
   
    g0=>P(0),
    g1=>P(1),
    d0=>c1,
    d1=>c2,
    d2=>c3,
    d3=>c0,
    y=>S
    );
    
  
   

c <= cat;
end Behavioral;
