library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity somador4 is
    Port ( P : in STD_LOGIC_VECTOR (3 downto 0);
           Q : in STD_LOGIC_VECTOR (3 downto 0);
           cin : in STD_LOGIC;
           S: out STD_LOGIC_VECTOR (3 downto 0);
           cout : out STD_LOGIC);
end somador4;

architecture Behavioral of somador4 is
-- declaracao do componente full_adder
component full_adder 
    Port ( a : in STD_LOGIC;
           b : in STD_LOGIC;
           cin : in STD_LOGIC;
           s : out STD_LOGIC;
           cout : out STD_LOGIC);
end component;
-- declaracao dos sinais (fios) internos que ligam os cin do bit i ao cout do bit i-1
signal C : std_logic_vector (4 downto 0);

begin
-- atribuicao do valor de C(0)
C(0) <= cin;

-- simplificacao (opcional) de utilizacao das
-- varias instancias
uGen1 : for i in 0 to 3 generate
    UFA: full_adder port map (
                a => P(i), b => Q(i), cin => C(i),
                s => S(i), cout =>C(i+1)
    );
end generate;
-- sinais adicionais de saida
cout <= C(4);

end Behavioral;
