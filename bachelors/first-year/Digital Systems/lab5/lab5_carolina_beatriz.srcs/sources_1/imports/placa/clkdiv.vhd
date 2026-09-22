library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

library UNISIM;
use UNISIM.VComponents.all;

entity clkdiv is
  Port (
    clk100M : in std_logic;
    clk1Hz : out std_logic);
end clkdiv;

architecture mixed of clkdiv is

  signal clk_i, clk_o : std_logic;
  signal cnt : std_logic_vector(27 downto 0);

begin

--  BUFG_INST1: BUFG port map (I => clk, O => clk_i);
--  clk50M <= clk_i;
  clk_i <= clk100M;
  -- Divide the master clock (100Mhz) down to an aprox 10Hz frequency.
  process (clk_i)
  begin
    if clk_i = '1' and clk_i'event then
      if cnt = X"2FAF07F" then         -- 50,000,000-1
        cnt <= (others => '0');
        clk_o <= not clk_o;
      else
        cnt <= cnt + 1;
      end if;
    end if;
  end process;

  -- BUFG: Global Clock Buffer (source by an internal signal)
  -- Xilinx HDL Language Template version 8.1i

  BUFG_INST2: BUFG port map (I => clk_o, O => clk1Hz);
  
end mixed;
