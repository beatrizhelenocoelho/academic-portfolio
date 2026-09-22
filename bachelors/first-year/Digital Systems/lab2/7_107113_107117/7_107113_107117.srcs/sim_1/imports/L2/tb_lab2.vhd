--------------------------------------------------------------
-- Numero do Grupo: <número do grupo>
-- <nome do 1º elemento do grupo> (<número do 1º elemento do grupo>)
-- <nome do 2º elemento do grupo> (<número do 2º elemento do grupo>)
--------------------------------------------------------------



library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity tb_lab2 is
--  Port ( );
end tb_lab2;

architecture Behavioral of tb_lab2 is
-- decalaracao do componente
component lab2 
    Port ( A : in STD_LOGIC_VECTOR (3 downto 0);
           P : in STD_LOGIC_VECTOR (1 downto 0);
           C : out STD_LOGIC_VECTOR (3 downto 0);
           S : out STD_LOGIC);
end component;
-- declaracao dos sinais para o testbench
signal A : STD_LOGIC_VECTOR (3 downto 0);
signal P : STD_LOGIC_VECTOR (1 downto 0);
signal C : STD_LOGIC_VECTOR (3 downto 0);
signal S : STD_LOGIC;

begin
-- declaracao da unidade de teste
Utest: lab2 port map (
            A => A, P => P,
            C => C, S => S );
        
-- descricao do gerador dos inputs P
 process
 begin
    P <= "00";
    wait for 16*10ns;
    P <= "01";
    wait for 16*10ns;
    P <= "10";
    wait for 16*10ns;
    P <= "11";
    wait for 16*10ns;
end process;

-- descricao do gerador dos inputs A
 process
 begin
    A <= "0000";
    wait for 10ns;
    A <= "0001";
    wait for 10ns;
    A <= "0010";
    wait for 10ns;
    A <= "0011";
    wait for 10ns;
    A <= "0100";
    wait for 10ns;
    A <= "0101";
    wait for 10ns;
    A <= "0110";
    wait for 10ns;
    A <= "0111";
    wait for 10ns;
    A <= "1000";
    wait for 10ns;
    A <= "1001";
    wait for 10ns;
    A <= "1010";
    wait for 10ns;
    A <= "1011";
    wait for 10ns;
    A <= "1100";
    wait for 10ns;
    A <= "1101";
    wait for 10ns;
    A <= "1110";
    wait for 10ns;
    A <= "1111";
    wait for 10ns;
    -- para completar !!!
end process;


end Behavioral;
