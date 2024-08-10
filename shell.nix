{ pkgs ? import <nixpkgs> {} }:

let
  fishConfig = pkgs.writeText "devshell-config.fish" ''
    functions -c fish_prompt original_fish_prompt
    function fish_prompt
      echo -n (set_color blue)"[dev] "(set_color normal)
      original_fish_prompt
    end
  '';
in
  pkgs.mkShell {
    buildInputs = with pkgs; [
      helix
      clang-tools # Includes clangd and clang-format
      lldb

      fish
      fzf
      ripgrep
    ];

    shellHook = ''
      export EDITOR=hx
      exec fish -C "source ${fishConfig}"
    '';

    interactiveShellInit = ''
      fzf_key_bindings
      set -U FZF_DEFAULT_OPTS "--height 40% --layout=reverse --border"
    '';
  }
