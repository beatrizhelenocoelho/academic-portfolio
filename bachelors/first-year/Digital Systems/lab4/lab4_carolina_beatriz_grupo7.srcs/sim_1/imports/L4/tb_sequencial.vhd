----------------------------------------------------------------------------------
--
-- Description: tb_sequencial.vhd
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

entity tb_sequencial is
--  Port ( );
end tb_sequencial;

architecture Behavioral of tb_sequencial is

signal M, CLK, INI: STD_LOGIC := '0';
signal Q: STD_LOGIC_VECTOR (2 downto 0) := "000";
signal B: STD_LOGIC_VECTOR (2 downto 0) := "011";

component sequencial is
    Port (
	   ini : in STD_LOGIC;
        clk : in STD_LOGIC;
        m : in STD_LOGIC;
        B : in STD_LOGIC_VECTOR (2 downto 0);
        Q : out STD_LOGIC_VECTOR (2 downto 0));
end component;

begin

SEQ : sequencial port map(
    ini => INI,
    clk => CLK,
    m => M,
    b => B,
    Q => Q
);

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

process
    begin
    M <= '0';
    wait for 400 ns;
    M <= '1';
    wait for 400 ns;
end process;

end Behavioral;
