
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity tb_lab0 is
--  Port ( );
end tb_lab0;

architecture Behavioral of tb_lab0 is
-- declaracao do componente
component lab0 
    Port ( a2 : in STD_LOGIC;
           a1 : in STD_LOGIC;
           a3 : in STD_LOGIC;
           f : out STD_LOGIC);
end component;
-- declaracao dos signais para o testbench
signal a3, a2, a1, f : std_logic;

begin
-- declaracao da instancia para teste
test_unit: lab0 port map ( 
                a3 => a3, a2 => a2, a1 => a1, 
                f => f);
                
process 
begin
  
    a2 <= '0';
    a1 <= '0';
    a3 <= '0';
    wait for 50 ns;
    a1 <= '1';
    wait for 50 ns;
    a2 <= '1';
    a1 <= '0';
    wait for 50 ns;
    a1 <= '1';
    wait for 50 ns;
    a1 <= '0';
    a2 <= '0';
    a3 <= '1';
    
    wait for 50 ns;
    
    a1 <= '1';
    wait for 50 ns;
    a1 <= '0';
    a2 <= '1';
    wait for 50 ns;
     a1 <= '1';
    wait;

end process;


end Behavioral;
