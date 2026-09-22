----------------------------------------------------------------------------------
--
-- Description: tb_sequencial2.vhd
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
USE ieee.std_logic_textio.ALL;

LIBRARY UNISIM;
USE UNISIM.Vcomponents.ALL;

LIBRARY std;
use std.textio.all;
use std.env.all;

entity tb_sequencial2 is
--  Port ( );
end tb_sequencial2;

architecture Behavioral of tb_sequencial2 is

type ROM is array (0 to 7) of STD_LOGIC_VECTOR (3 downto 0);

-- declaracao do componente
component sequencial2 
	Port (ini : in STD_LOGIC;
          clk : in STD_LOGIC;
          Q : out STD_LOGIC_VECTOR (2 downto 0);
          S : out STD_LOGIC_VECTOR (3 downto 0));
end component;
-- declaracao dos signais para o testbench
signal ini, clk : STD_LOGIC := '0';
signal S: std_logic_vector (3 downto 0);
signal Q: std_logic_vector (2 downto 0);



begin
-- declaracao da instancia para teste
test_unit: sequencial2 port map ( 
            ini => ini, 
            clk => CLK, 
            Q => Q,
            S => S );

-- gerador dos sinais
process 
    begin
    wait for 5 ns;
    CLK <= '0'; wait for 10 ns; 
    CLK <= '1'; wait for 5 ns;
end process;

process
    begin
    INI <= '1';
    wait for 20 ns;
    INI <= '0';
    wait;
end process;


-- Modulo para executar o teste automático e mostrar 
-- as mensagems na consola do simulador.
-- NOTA: Este codigo é apenas válido para simulaçao e NÂO pode ser 
-- usado na definiçao de circuitos na disciplina de Sistema Digitais
PROCESS
	variable intro : line;
   BEGIN -- print once
      WAIT FOR 1ns; 
	    write(intro, string'("")); writeline(OUTPUT, intro); 
	    write(intro, string'(" Sistemas Digitais - Lab 3")); writeline(OUTPUT, intro); 
		write(intro, string'(" Resultados do teste")); writeline(OUTPUT, intro);
		write(intro, string'("")); writeline(OUTPUT, intro);
	  WAIT for 195ns;
	    write(intro, string'("Teste Completo")); writeline(OUTPUT, intro);
	  finish(0);		
END PROCESS;	


PROCESS
     variable my_line : line;
	 BEGIN 
      WAIT FOR 10ns; 
       if INI = '0' then
        write(my_line, string'("S = "));
        write(my_line, S) ;
        write(my_line, string'(", Q = "));
        write(my_line, Q) ;
        writeline(OUTPUT, my_line); 	
      	
	  end if;	
	  WAIT FOR 10ns;
 END PROCESS;	
 
end Behavioral;
