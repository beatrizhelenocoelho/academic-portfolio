----------------------------------------------------------------------------------
-- descodificador 3:8 com saidas activas a '0' 
----------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity decoder_neg is
    Port ( I : in STD_LOGIC_VECTOR (2 downto 0);
           en : in STD_LOGIC;
             O : out STD_LOGIC_VECTOR (7 downto 0));
end decoder_neg;

-- decoder with 3 bit input and 8 negative outputs
architecture Behavioral of decoder_neg is
    signal ni: STD_LOGIC_VECTOR (2 downto 0);
begin
	nI <= not (I);	
	
	-- maxterm based decoder
	O(0)  <=   I(0) or  I(1) or  I(2) or not(en);
	O(1)  <=  nI(0) or  I(1) or  I(2) or not(en);
	O(2)  <=   I(0) or nI(1) or  I(2) or not(en);
	O(3)  <=  nI(0) or nI(1) or  I(2) or not(en);
	O(4)  <=   I(0) or  I(1) or nI(2) or not(en);
	O(5)  <=  nI(0) or  I(1) or nI(2) or not(en);
	O(6)  <=   I(0) or nI(1) or nI(2) or not(en);
	O(7)  <=  nI(0) or nI(1) or nI(2) or not(en);
	
	
end Behavioral;
