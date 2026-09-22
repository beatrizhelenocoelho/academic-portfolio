----------------------------------------------------------------------------------
-- descodificador 3:8 com saidas activas a '1' 
----------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity decoder is
    Port ( I : in STD_LOGIC_VECTOR (2 downto 0);
           en : in STD_LOGIC;
             O : out STD_LOGIC_VECTOR (7 downto 0));
end decoder;

-- decoder with 3 bit input and 8 positive outputs
architecture Behavioral of decoder is
    signal ni: STD_LOGIC_VECTOR (2 downto 0);
begin
	nI <= not (I);	
	
	-- minterm based decoder
	O(7)  <=   I(0) and  I(1) and  I(2) and en;
	O(6)  <=  nI(0) and  I(1) and  I(2) and en;
	O(5)  <=   I(0) and nI(1) and  I(2) and en;
	O(4)  <=  nI(0) and nI(1) and  I(2) and en;
	O(3)  <=   I(0) and  I(1) and nI(2) and en;
	O(2)  <=  nI(0) and  I(1) and nI(2) and en;
	O(1)  <=   I(0) and nI(1) and nI(2) and en;
	O(0)  <=  nI(0) and nI(1) and nI(2) and en;
	
	
	
end Behavioral;
