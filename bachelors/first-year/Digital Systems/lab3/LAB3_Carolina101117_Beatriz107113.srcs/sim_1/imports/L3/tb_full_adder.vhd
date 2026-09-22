--------------------------------------------------------------
-- Numero do Grupo: <número do grupo>
-- <nome do 1º elemento do grupo> (<número do 1º elemento do grupo>)
-- <nome do 2º elemento do grupo> (<número do 2º elemento do grupo>)
--------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;


entity tb_full_adder is
--  Port ( );
end tb_full_adder;

architecture Behavioral of tb_full_adder is
-- declaracao do componente
component full_adder 
    Port ( a : in STD_LOGIC;
           b : in STD_LOGIC;
           cin : in STD_LOGIC;
           s : out STD_LOGIC;
           cout : out STD_LOGIC);
end component;
-- declaracao dos signais para o testbench
signal a, b, cin, s, cout : std_logic;
begin
-- declaracao da instancia para teste
test_unit: full_adder port map ( 
                a => a, b => b, cin => cin,
                s => s, cout => cout);

-- gerador dos sinais - combinacoes de a, b e cin
process 
begin
    cin <= '0';
    
    a <= '0'; 
    b <= '0';
    wait for 50 ns; 
    a <= '0';
    b <= '1';
    wait for 50 ns; 
    a <= '1'; 
    b <= '0';
    wait for 50 ns; 
    a <= '1';
    b <= '1';
    wait for 50 ns; 
    
    cin <= '1';
        
    a <= '0'; 
    b <= '0';
    wait for 50 ns; 
    a <= '0';
    b <= '1';
    wait for 50 ns; 
    a <= '1'; 
    b <= '0';
    wait for 50 ns; 
    a <= '1';
    b <= '1';
    wait for 50 ns;
end process;


end Behavioral;
