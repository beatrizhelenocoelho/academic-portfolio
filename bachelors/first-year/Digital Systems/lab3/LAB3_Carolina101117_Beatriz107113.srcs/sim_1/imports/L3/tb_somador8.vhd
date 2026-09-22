----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 10/19/2017 02:18:25 PM
-- Design Name: 
-- Module Name: tb_somador8 - Behavioral
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


entity tb_somador8 is
--  Port ( );
end tb_somador8;

architecture Behavioral of tb_somador8 is
-- declaracao do componente
component somador8 
    Port ( P : in STD_LOGIC_VECTOR (7 downto 0);
           Q : in STD_LOGIC_VECTOR (7 downto 0);
           cin : in STD_LOGIC;
           R : out STD_LOGIC_VECTOR (7 downto 0);
           cout : out STD_LOGIC);
end component;
-- declaracao dos signais para o testbench
signal A, B : std_logic_vector (7 downto 0);
signal cin : std_logic;
signal R : std_logic_vector (7 downto 0);
signal cout : std_logic;

begin
-- declaracao da instancia para teste
test_unit: somador8 port map ( 
                P => A, Q => B, cin => cin,
                R => R, cout => cout);

-- combinacoes para testar o circuito 
process
begin
    -- somador R=P+Q+cin
    -- test values: A=73, B=37
    
    -- operacao: A+B 
    -- P=A(73), Q=B(37), cin=0
    -- resultado esperado: R=110, cout=0
    A <= "01001001";
    B <= "00100101";
    cin <= '0';
    wait for 100ns;
    
    -- operacao: A-B 
    -- P=A(73), Q=B em complemento para 2 (-37), cin=0
    -- resultado esperado: R=36, cout=1
    A <= "01001001";
    B <= "11011011";
    cin <= '0';
    wait for 100ns;
    
    -- operacao: A+5 
    -- P=A(73), Q=1, cin=0
    -- resultado esperado: R=78, cout=0
    A <= "01001001";
    B <= "00000101";
    cin <= '0';
    wait for 100ns;
    
    -- operacao: B+1 
    -- P=B(37), Q=0, cin=1
    -- resultado esperado: R=38, cout=0
    A <= "00100101";
    B <= "00000000";
    cin <= '1';
    wait for 100ns;
    
    wait;
end process;

end Behavioral;
