----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 10/03/2018 01:32:41 PM
-- Design Name: 
-- Module Name: tb_l3_all - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
USE ieee.std_logic_textio.ALL;

LIBRARY UNISIM;
USE UNISIM.Vcomponents.ALL;

LIBRARY std;
use std.textio.all;
use std.env.all;


entity tb_lab3_all is
--  Port ( );
end tb_lab3_all;

architecture Behavioral of tb_lab3_all is
-- declaracao do componente
component lab3 
    Port ( A : in STD_LOGIC_VECTOR (4 downto 0);
           B : in STD_LOGIC_VECTOR (4 downto 0);
           I : in STD_LOGIC;
           S : out STD_LOGIC_VECTOR (7 downto 0));
end component;

-- declaracao dos signais para o testbench
signal test_in : std_logic_vector (10 downto 0) := "00000000000";
signal errors : std_logic_vector (10 downto 0) := "00000000000";
signal A : std_logic_vector (4 downto 0);
signal B : std_logic_vector (4 downto 0);
signal I : STD_LOGIC;
signal S : std_logic_vector (7 downto 0);

begin
-- declaracao da instancia para teste
test_unit: lab3 port map ( 
            A => A, B => B, I => I, S => S );

 I <= test_in(10);
 A <= test_in(9 downto 5);  
 B <= test_in(4 downto 0); 


-- Modulo para executar o teste automático e mostrar 
-- as mensagems na consola do simulador.
-- NOTA: Este codigo é apenas válido para simulaçao e NÂO pode ser 
-- usado na definiçao de circuitos na disciplina de Sistema Digitais

process
    variable counter : unsigned(10 downto 0) := "00000000000";
begin
    wait for 10ps;
    counter := counter+ 1; 
    test_in <= std_logic_vector(counter);
end process;

PROCESS
	variable intro : line;
   BEGIN -- print once
      WAIT FOR 1ps; 
	    write(intro, string'("")); writeline(OUTPUT, intro); 
	    write(intro, string'(" Sistemas Digitais - Lab 2")); writeline(OUTPUT, intro); 
		write(intro, string'(" Resultados do teste longo:")); writeline(OUTPUT, intro);
        write(intro, string'(" (Apenas os erros sao mostrados)")); writeline(OUTPUT, intro);
		write(intro, string'("")); writeline(OUTPUT, intro);
	  WAIT for 20475ps;
	  write(intro, string'(" Teste Completo!!")); writeline(OUTPUT, intro);
	  write(intro, string'(" TOTAL de ERROS "));
      write(intro, to_integer(signed(errors))); writeline(OUTPUT, intro);
	  write(intro, string'("")); writeline(OUTPUT, intro);
	  finish(0);		
END PROCESS;	

PROCESS
    variable my_line : line;
	variable OP1 : signed (7 downto 0);
    variable OP2 : signed (7 downto 0);
    variable S_ref2 : std_logic_vector (7 downto 0);
    variable error_counter : unsigned(10 downto 0) := "00000000000";
   BEGIN -- print every 5 ns
      WAIT FOR 2ps; 
      
      -- modelo de simulaçao das operações
      OP1 := shift_right(to_signed((to_integer(signed(B)) - to_integer(signed(A))),8),3);
      OP2 := shift_right(to_signed(4*to_integer(signed(A)) + 3*to_integer(signed(B)),8),2);
      if I = '0' then
        S_ref2 :=  std_logic_vector(OP1);
       else
        S_ref2 :=  std_logic_vector(OP2);
      end if;
      
	  if not((S xor S_ref2) = "00000000") then 
       write(my_line, string'("ERRO : I = "));
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
		error_counter := error_counter+ 1; 
	 end if;		
		WAIT FOR 8ps;	
		 errors <= std_logic_vector(error_counter);
   END PROCESS;	
end Behavioral;
