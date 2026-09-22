----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 10/19/2017 02:09:49 PM
-- Design Name: 
-- Module Name: somador8 - Behavioral
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

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity somador8 is
    Port ( P : in STD_LOGIC_VECTOR (7 downto 0);
           Q : in STD_LOGIC_VECTOR (7 downto 0);
           cin : in STD_LOGIC;
           R : out STD_LOGIC_VECTOR (7 downto 0);
           cout : out STD_LOGIC);
end somador8;

architecture Behavioral of somador8 is
-- declaracao do componente somador4
component somador4 
    Port ( P : in STD_LOGIC_VECTOR (3 downto 0);
           Q : in STD_LOGIC_VECTOR (3 downto 0);
           cin : in STD_LOGIC;
           S : out STD_LOGIC_VECTOR (3 downto 0);
           cout : out STD_LOGIC);
end component;

-- declaracao do sinais interno 
signal som1_P, som1_Q, som1_S : STD_LOGIC_VECTOR (3 downto 0);
signal som1_cin, som1_cout : STD_LOGIC;

signal som2_P, som2_Q, som2_S : STD_LOGIC_VECTOR (3 downto 0);
signal som2_cin, som2_cout : STD_LOGIC;

begin
-- instancia do SOM1 (port map)
SOM1: somador4 port map (
        P => som1_P, Q => som1_Q, cin => som1_cin,
        S => som1_S, cout => som1_cout
    );

-- instancia do SOM2 (port map)
SOM2: somador4 port map (
        P => som2_P, Q => som2_Q, cin => som2_cin,
        S => som2_S, cout => som2_cout
    );

-- Attribui��o das entradas do somador 1 
-- DEVE COMPLETAR

    som1_P   <= P(3) & P(2) & P(1) & P(0);
    som1_Q   <= Q(3) & Q(2) & Q(1) & Q(0);
    som1_cin <= cin;
    
-- Attribui��o das entradas do somador 2 
-- DEVE COMPLETAR
    som2_P   <= P(7) & P(6) & P(5) & P(4);
    som2_Q   <= Q(7) & Q(6) & Q(5) & Q(4);
    som2_cin <= som1_cout;  
-- Attribui��o das saidas do somador8
-- DEVE COMPLETAR 
    cout <= som2_cout;
    R    <= som2_S & som1_S;--- isto é o resultado das somas de cada quatro em quatro bits, sendo os primeiros de 7, 6, 5... e depois 4..

end Behavioral;
