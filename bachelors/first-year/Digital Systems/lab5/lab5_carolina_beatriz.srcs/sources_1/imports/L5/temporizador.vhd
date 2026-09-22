----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 10.11.2016 10:47:20
-- Design Name: 
-- Module Name: float_add - Behavioral
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
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity temporizador is
    Port ( clk       : in STD_LOGIC;
           timer_en  : in STD_LOGIC;
           timer_val : in STD_LOGIC_VECTOR (3 downto 0);
           timer_cnt : out STD_LOGIC_VECTOR (3 downto 0);
           TOUT      : out STD_LOGIC);
end temporizador;

architecture Behavioral of temporizador is
    -- sinais
    signal cnt : STD_LOGIC_VECTOR (3 downto 0) := (others => '0');
    signal timer_term : STD_LOGIC_VECTOR (3 downto 0) := (others => '1');
    signal ce, rst : STD_LOGIC;
    
begin

  ce <= timer_en; -- when cnt = "0000" else '1';
  rst <= not timer_en;
  
  process (clk)
  begin
    if clk='1' and clk'event then
      if ce = '1' then
        timer_term <= timer_val;
      end if; 
    end if;
  end process;
    
  process (clk)
    begin
       if clk='1' and clk'event then
         if (rst = '1') then
           cnt <= "0000";
         elsif ce = '1' then
           if cnt = timer_term then
             cnt <= "0000";
           else
             cnt <= cnt + 1;
           end if;
         end if;
       end if;
    end process;

  TOUT <= '1' when cnt = timer_term and ce = '1' else '0';
  timer_cnt <= cnt when ce = '1' else "0000";
 
end Behavioral;
