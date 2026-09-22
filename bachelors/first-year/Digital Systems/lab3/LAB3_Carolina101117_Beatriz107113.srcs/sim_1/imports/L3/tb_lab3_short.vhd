--------------------------------------------------------------
-- Numero do Grupo: <número do grupo>
-- <nome do 1º elemento do grupo> (<número do 1º elemento do grupo>)
-- <nome do 2º elemento do grupo> (<número do 2º elemento do grupo>)
--------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
USE ieee.std_logic_textio.ALL;

LIBRARY UNISIM;
USE UNISIM.Vcomponents.ALL;

LIBRARY std;
use std.textio.all;
use std.env.all;

entity tb_lab3_short is
--  Port ( );
end tb_lab3_short;

architecture Behavioral of tb_lab3_short is
-- declaracao do componente
component lab3 
    Port ( A : in STD_LOGIC_VECTOR (4 downto 0);
           B : in STD_LOGIC_VECTOR (4 downto 0);
           I : in STD_LOGIC;
           S : out STD_LOGIC_VECTOR (7 downto 0));
end component;
-- declaracao dos signais para o testbench
signal A : std_logic_vector (4 downto 0);
signal B : std_logic_vector (4 downto 0);
signal I : STD_LOGIC;
signal S : std_logic_vector (7 downto 0);

begin
-- declaracao da instancia para teste
test_unit: lab3 port map ( 
            A => A, B => B, I => I, S => S );

-- gerador dos sinais
process
begin
    I <= '0';
    A <= "10010";  
    B <= "10011"; 
    wait for 100ns;
    
    I <= '1';
    A <= "10010";  
    B <= "10011"; 
    wait for 100ns;  
    
    I <= '0';
    A <= "10010";  
    B <= "01011"; 
    wait for 100ns;
    
    I <= '1';
    A <= "10010";  
    B <= "01011"; 
    wait for 100ns;
    finish(0);
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
	    write(intro, string'(" Sistemas Digitais - Lab 2")); writeline(OUTPUT, intro); 
		write(intro, string'(" Resultados do teste curto:")); writeline(OUTPUT, intro);
		write(intro, string'("")); writeline(OUTPUT, intro);
	  WAIT for 395ns;
	    write(intro, string'("Teste Completo")); writeline(OUTPUT, intro);
	  WAIT;		
END PROCESS;	

PROCESS
    variable my_line : line;
	variable OP1 : signed (7 downto 0);
    variable OP2 : signed (7 downto 0);
    variable S_ref2 : std_logic_vector (7 downto 0);
   BEGIN -- print every 5 ns
      WAIT FOR 10ns; 
      OP1 := shift_right(to_signed((to_integer(signed(B)) - to_integer(signed(A))),8),3);
      OP2 := shift_right(to_signed(4*to_integer(signed(A)) + 3*to_integer(signed(B)),8),2);
      if I = '0' then
        S_ref2 :=  std_logic_vector(OP1);
       else
        S_ref2 :=  std_logic_vector(OP2);
      end if;
      
	  if (S xor S_ref2) = "00000000" then write(my_line, string'("OK   : I = "));
      else write(my_line, string'("ERRO : I = "));
	  end if;
	    write(my_line, I) ;
	    write(my_line, string'(", A = "));
		write(my_line, A) ;
		write(my_line, string'("b ["));
		write(my_line, to_integer(signed(A)));
		write(my_line, string'("], B = "));
		write(my_line, B) ;
		write(my_line, string'("b ["));
		write(my_line, to_integer(signed(B)));
		write(my_line, string'("]; S = "));
		write(my_line, S);
		write(my_line, string'("b ["));
		write(my_line, to_integer(signed(S)));
		write(my_line, string'("], expect = "));
		write(my_line, S_ref2);
        write(my_line, string'("b ["));
        write(my_line, to_integer(signed(S_ref2)));
        write(my_line, string'("]"));
		writeline(OUTPUT, my_line); 		
		WAIT FOR 90ns;	
   END PROCESS;	



end Behavioral;
