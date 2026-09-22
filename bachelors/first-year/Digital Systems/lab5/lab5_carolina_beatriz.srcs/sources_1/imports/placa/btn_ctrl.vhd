library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

---- Uncomment the following library declaration if instantiating
---- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity btn_ctrl is
  port (
    clk, rst : in std_logic;
    btnI : in std_logic_vector (4 downto 0);
    btnO : out std_logic_vector (4 downto 0));
end btn_ctrl;

architecture Behavioral of btn_ctrl is
  type fsm_states is ( s_initial, s_wait, s_exe );
  signal currstate, nextstate: fsm_states;
  signal btnR : std_logic_vector (4 downto 0);
  signal cnt : std_logic_vector(27 downto 0);
  signal regb_ce, cnt_ce, cnt_rst : std_logic;

begin
  state_reg: process (clk)
  begin 
    if clk'event and clk = '1' then
      if rst = '1' then
        currstate <= s_initial ;
      else
        currstate <= nextstate ;
      end if ;
    end if ;
  end process;

  reg_btn: process (clk)
  begin 
    if clk'event and clk = '1' then
      if regb_ce = '1' then
        btnR <= btnI ;
      end if ;
    end if ;
  end process;
  
  state_comb: process (currstate, btnI, btnR, cnt)
  begin  --  process
    nextstate <= currstate ;  
    -- by default, does not change the state.
    regb_ce <= '0';
    cnt_rst <= '0';
    cnt_ce <= '0';
      
    case currstate is
      when s_initial =>
        if btnI /= "00000" then
          nextstate <= s_exe ;
          regb_ce <= '1';
        end if;
        btnO <= "00000";
        cnt_rst <= '1';
        
      when s_exe =>
        btnO <= btnR;
        cnt_ce <= '1';
        if cnt = X"5F5E0FF" then         -- 100,000,000-1
          nextstate <= s_wait ;
        end if;
        
      when s_wait =>
        if btnI = "00000" then
          nextstate <= s_initial;
        end if;
        btnO <= "00000";

    end case;
  end process;

  counter: process (clk)
    begin
       if clk='1' and clk'event then
         if (cnt_rst = '1') then
           cnt <= (others => '0');
         elsif cnt_ce = '1' then
             cnt <= cnt + 1;
         end if;
       end if;
    end process;
end Behavioral;

