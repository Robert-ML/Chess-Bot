#include "game.h"

Game::Game(char wb) {
  make_print_board_matrix(*this, wb);

  // init my color, board and vector of pieces
  m_color = wb;
  m_board = get_initial_board_matrix(m_color);
	m_pieces = get_initial_positions(m_color);

  // init enemy's color, board and vector of pieces
  if (wb == WHITE) { // if we are white
    e_color = BLACK; // enemy is black
  } else {       // if we are black
    e_color = WHITE; // enemy is white
  }
  e_board = get_initial_board_matrix(e_color);
  e_pieces = get_initial_positions(e_color);
}

// ------------------------- find next move for game --------------------------

  std::vector<char> Game::find_next_move() {
  // {src_x, src_y, dst_x, dst_y, piece_type, priority_code}
    std::vector<std::vector<char>> all_moves = get_moves_m(*this);
    std::sort(all_moves.begin(), all_moves.end(), order_moves_by_priority);

    std::vector<char> chosen_move;
    int alpha = LOW;
    int beta = HIGH;

    for (auto &move : all_moves) {
      int score;
      // holds info to undo the move later: {piece_taken}
      std::vector<char> info = this->apply_move_m(move);

      if (this->is_check_m) {
        score = LOW - 1;
      } else {
        score = alphabeta_mini(DEPTH - 1, alpha, beta, *this);
      }

      if (score = HIGH) { // if we find this it means we will give checkmate
        std::cout << "#ATENTIE: Am gasit un mod de a castiga cu miscarea: (" << move[0] << ", " << move[1] << ") -> (" << move[2] 
          << ", " << move[3] << "); piece: " << this->m_board[move[0]][move[1]] << " -> " << this->m_board[move[2]][move[3]] << std::endl;
          return move;
      } else if (score > alpha) {
        alpha = score;
        chosen_move = move; // we found a better move than losing
      }

      this->undo_move_e(info, move);
    }
    if (chosen_move.size() == 0) {
      std::cout << "#ATENTION: We couldn't find a move that saves our ass and we will get chechmated :(" << std::endl;
    }
    return chosen_move;
  }


// --------------------------- apply move functions ---------------------------

// calls apply_move
std::vector<char> Game::apply_move_m(std::vector<char> &move) {
  return apply_move(move, m_board, m_pieces, e_board, e_pieces);
}

// calls apply_move
std::vector<char> Game::apply_move_e(std::vector<char> &move) {
  return apply_move(move, e_board, e_pieces, m_board, m_pieces);
}

// more info about the funtion in the header file
// return value: {piece_taken}
std::vector<char> apply_move(std::vector<char> &move, std::vector<std::vector<char>> &p_board,std::vector<std::vector<char>> &p_pieces,
std::vector<std::vector<char>> &h_board, std::vector<std::vector<char>> &h_pieces) {
  // structure of move: {src_x, src_y, dst_x, dst_y, piece_type, priority_code} | only the first 4 fields in the vector are mandatory
  char src_x = move[0];
  char src_y = move[1];
  char dst_x = move[2];
  char dst_y = move[3];
  std::vector<char> info;

  // change the personal vector of pieces
  for (auto &v : p_pieces) {
    if (v[0] == src_x && v[1] == src_y) {
      v[0] = dst_x;
      v[1] = dst_y;
      break;
    }
  }
  // change our board
  info.push_back(p_board[dst_x][dst_y]); // save the info in the cell
  p_board[dst_x][dst_y] = p_board[src_x][src_y];
  p_board[src_x][src_y] = EMPTY_CELL;


  // prepare to change the enemy's information
  char ep_src_x, ep_src_y; // enemy's perspective source x/y
  change_coordonates(src_x, src_y, ep_src_x, ep_src_y); // get the source coordonates from the enemy's perspective
  char ep_dst_x, ep_dst_y; // enemy's perspective destination x/y
  change_coordonates(dst_x, dst_y, ep_dst_x, ep_dst_y); // get the destination coordonates from the enemy's perspective

  // change enemy's board
  h_board[ep_dst_x][ep_dst_y] = h_board[ep_src_x][ep_src_y];
  h_board[ep_src_x][ep_src_y] = EMPTY_CELL;

  // if an enemy piece was taken we update his vector of pieces
  if (info[0] > 10) {
    // change the enemy's vector of pieces
    for (auto &v : h_pieces) {
      if (v[0] == ep_dst_x && v[1] == ep_dst_y) {
        v[0] = -1;
        v[1] = -1;
        break;
      }
    }
  } else if (info[0] != EMPTY_CELL){ // if something is wrong
    std::cout << "#ERROR: apply_move illogical value in cell: " << info[0] << std::endl;
    info.push_back(-1);
  }

  return info;
}

// --------------------------- undo move functions ----------------------------

//calls undo_move
void Game::undo_move_m(std::vector<char> &info, std::vector<char> &move) {
  undo_move(info, move, m_board, m_pieces, e_board, e_pieces);
}

//calls undo_move
void Game::undo_move_e(std::vector<char> &info, std::vector<char> &move) {
  undo_move(info, move, e_board, e_pieces, m_board, e_pieces);
}

// more info about the funtion in the header file
void undo_move(std::vector<char> &info, std::vector<char> &move, std::vector<std::vector<char>> &p_board,std::vector<std::vector<char>> &p_pieces,
std::vector<std::vector<char>> &h_board, std::vector<std::vector<char>> &h_pieces) {
  // structure of move: {src_x, src_y, dst_x, dst_y, piece_type, priority_code} | only the first 4 fields in the vector are mandatory
  char src_x = move[0];
  char src_y = move[1];
  char dst_x = move[2];
  char dst_y = move[3];

  // change the personal vector of pieces
  for (auto &v : p_pieces) {
    if (v[0] == dst_x && v[1] == dst_y) {
      v[0] = src_x;
      v[1] = src_y;
      break;
    }
  }
  // change our board
  p_board[src_x][src_y] = p_board[dst_x][dst_y];
  p_board[dst_x][dst_y] = info[0]; // save enemy's information


  // prepare to change the enemy's information
  char ep_src_x, ep_src_y; // enemy's perspective source x/y
  change_coordonates(src_x, src_y, ep_src_x, ep_src_y); // get the source coordonates from the enemy's perspective
  char ep_dst_x, ep_dst_y; // enemy's perspective destination x/y
  change_coordonates(dst_x, dst_y, ep_dst_x, ep_dst_y); // get the destination coordonates from the enemy's perspective
  char ep_taken_piece = info[0] - 10;

  // change enemy's board
  h_board[ep_src_x][ep_src_y] = h_board[ep_dst_x][ep_dst_y];
  h_board[ep_dst_x][ep_dst_y] = info[0];

  // if one of enemy's pieces was taken, now we put it's info back in the enemy's vector of pieces at the first aviable spot
  if (info[0] > 10) {
    // restitute the enemy's vector of pieces
    for (auto &v : h_pieces) {
      if (v[0] == -1 && v[1] == -1 && v[2] == ep_taken_piece) {
        v[0] = ep_dst_x;
        v[1] = ep_dst_y;
        break;
      }
    }
  } else if (info[0] != EMPTY_CELL) {
    std::cout << "#ERROR: undo_move illogical value in info: " << info[0] << std::endl;
  }
}

// --------------------------- chess check and attack funcitons --------------------------

// all pieces that can attack my piece
// return a vector of {x, y, piece type}
std::vector<std::vector<char>> check_attackers(char i, char j, std::vector<std::vector<char>> &chess_board) {
  std::vector<std::vector<char>> possible_attackers;
  int cod_piesa = chess_board[i][j];
  // pawn
  if (check_check_validity(i + 1, j + 1, chess_board) == 2 && chess_board[i + 1][j + 1] == 11) {
    possible_attackers.push_back(attacks(i + 1, j + 1, chess_board));
  }
  if (check_check_validity(i + 1, j - 1, chess_board) == 2 && chess_board[i + 1][j - 1] == 11) {
    possible_attackers.push_back(attacks(i + 1, j - 1, chess_board));
  }
  // rook and quuen for lines
  for(auto d : all_directions) {
    if (d != up || d != dw || d != rg || d != lf) continue;
    char x = d.first;
    char y = d.second;
    int ok = 0;
    int temp = 1;
    while(temp != 0 && ok == 0) {
      temp = check_check_validity(i + x, j + y, chess_board);
      if (temp == 2 && (chess_board[i + x][j + y] == 12 || chess_board[i + x][j + y] == 16)) {
        possible_attackers.push_back(attacks(i + x, j + y, chess_board));
        ok = 1;
      } else if (temp == 0) {
        break;
      } else {
        i += x;
        j += y;
      }
    }
  }
  // knight 
  if (check_check_validity(i + 1, j + 2, chess_board) == 2 && chess_board[i + 1][j + 2] == 13) {
    possible_attackers.push_back(attacks(i + 1, j + 2, chess_board));
  }
  if (check_check_validity(i + 2, j + 1, chess_board) == 2 && chess_board[i + 2][j + 1] == 13) {
    possible_attackers.push_back(attacks(i + 2, j + 1, chess_board));
  }
  if (check_check_validity(i + 1, j - 2, chess_board) == 2 && chess_board[i + 1][j - 2] == 13) {
    possible_attackers.push_back(attacks(i + 1, j - 2, chess_board));
  }
  if (check_check_validity(i + 2, j - 1, chess_board) == 2 && chess_board[i + 2][j - 1] == 13) {
    possible_attackers.push_back(attacks(i + 2, j - 1, chess_board));
  }
  if (check_check_validity(i - 1, j + 2, chess_board) == 2 && chess_board[i - 1][j + 2] == 13) {
    possible_attackers.push_back(attacks(i - 1, j + 2, chess_board));
  }
  if (check_check_validity(i - 2, j + 1, chess_board) == 2 && chess_board[i - 2][j + 1] == 13) {
    possible_attackers.push_back(attacks(i - 2, j + 1, chess_board));
  }
  if (check_check_validity(i - 1, j - 2, chess_board) == 2 && chess_board[i - 1][j - 2] == 13) {
    possible_attackers.push_back(attacks(i - 1, j - 2, chess_board));
  }
  if (check_check_validity(i - 2, j - 1, chess_board) == 2 && chess_board[i - 2][j - 1] == 13) {
    possible_attackers.push_back(attacks(i - 2, j - 1, chess_board));
  }
  // bishop (white and black) and quuen for diagonals
  for(auto d : all_directions) {
    if (d != ur || d != ul || d != dr || d != dl) continue;
    char x = d.first;
    char y = d.second;
    int ok = 0;
    int temp = 1;
    while(temp != 0 && ok == 0) {
      temp = check_check_validity(i + x, j + y, chess_board);
      if (temp == 2 && (chess_board[i + x][j + y] == 14 || chess_board[i + x][j + y] == 15 || chess_board[i + x][j + y] == 16)) {
        possible_attackers.push_back(attacks(i + x, j + y, chess_board));
        ok = 1;
      } else if (temp == 0) {
        break;
      } else {
        i += x;
        j += y;
      }
    }
  }
  // king 
  for (auto d: all_directions) {
    char x = d.first;
    char y = d.second;
    if ((check_check_validity(i + x, j + y, chess_board) == 2) && chess_board[i + x][j + y] == 17) {
        possible_attackers.push_back(attacks(i + x, j + y, chess_board));
    }
  }
}

// possible ways of what a piece can attack
// return a vector of {x, y, piece type}
std::vector<std::vector<char>> check_attack(char i, char j, std::vector<std::vector<char>> &chess_board) {
  std::vector<std::vector<char>> possible_attacks;
  int cod_piesa = chess_board[i][j];
  switch (cod_piesa) {
    case 1: // pawn
      if (check_validity(i + 1, j + 1, chess_board) >= 2) {
        possible_attacks.push_back(attacks(i + 1, j + 1, chess_board));
      }
      if (check_validity(i + 1, j - 1, chess_board) >= 2) {
        possible_attacks.push_back(attacks(i + 1, j - 1, chess_board));
      }
      break;
    
    case 2: // rook
      for(auto d : all_directions) {
        if (d != up || d != dw || d != rg || d != lf) continue;
        char x = d.first;
        char y = d.second;
        int ok = 0;
        int temp = 1;
        while(temp != 0 && ok == 0) {
          temp = check_check_validity(i + x, j + y, chess_board);
          if (temp == 2) {
            possible_attacks.push_back(attacks(i + x, j + y, chess_board));
            ok = 1;
          } else if (temp == 0) {
            break;
          } else {
            i += x;
            j += y;
          }
        }
      }
      break;

    case 3: // knight
      if (check_validity(i + 1, j + 2, chess_board) >= 2) {
        possible_attacks.push_back(attacks(i + 1, j + 2, chess_board));
      }
      if (check_validity(i + 2, j + 1, chess_board) >= 2) {
       possible_attacks.push_back(attacks(i + 2, j + 1, chess_board));
      }
      if (check_validity(i + 1, j - 2, chess_board) >= 2) {
        possible_attacks.push_back(attacks(i + 1, j - 2, chess_board));
      }
      if (check_validity(i + 2, j - 1, chess_board) >= 2) {
        possible_attacks.push_back(attacks(i + 2, j - 1, chess_board));
      }
      if (check_validity(i - 1, j + 2, chess_board) >= 2) {
        possible_attacks.push_back(attacks(i - 1, j + 2, chess_board));
      }
      if (check_validity(i - 2, j + 1, chess_board) >= 2) {
        possible_attacks.push_back(attacks(i - 2, j + 1, chess_board));
      }
      if (check_validity(i - 1, j - 2, chess_board) >= 2) {
        possible_attacks.push_back(attacks(i - 1, j - 2, chess_board));
      }
      if (check_validity(i - 2, j - 1, chess_board) >= 2) {
        possible_attacks.push_back(attacks(i - 2, j - 1, chess_board));
      }
      break;

    case 4: // black bishop
    case 5: // white bishop
      for(auto d : all_directions) {
        if (d != ur || d != ul || d != dr || d != dl) continue;
        char x = d.first;
        char y = d.second;
        int ok = 0;
        int temp = 1;
        while(temp != 0 && ok == 0) {
          temp = check_check_validity(i + x, j + y, chess_board);
          if (temp == 2) {
            possible_attacks.push_back(attacks(i + x, j + y, chess_board));
            ok = 1;
          } else if (temp == 0) {
            break;
          } else {
            i += x;
            j += y;
          }
        }
      }
      break;

    case 6: // queen
      for(auto d : all_directions) {
        char x = d.first;
        char y = d.second;
        int ok = 0;
        int temp = 1;
        while(temp != 0 && ok == 0) {
          temp = check_check_validity(i + x, j + y, chess_board);
          if (temp == 2) {
            possible_attacks.push_back(attacks(i + x, j + y, chess_board));
            ok = 1;
          } else if (temp == 0) {
            break;
          } else {
            i += x;
            j += y;
          }
        }
      }
      break;

    case 7: // king
      for (auto d: all_directions) {
        char x = d.first;
        char y = d.second;
        if ((check_check_validity(i + x, j + y, chess_board) == 2)) {
            possible_attacks.push_back(attacks(i + x, j + y, chess_board));
        }
      }
      break;
    
    default:
      break;
  }
  return possible_attacks;
}

std::vector<char> attacks(char i, char j, std::vector<std::vector<char>> &chess_board) {
  int type = chess_board[i][j];
  std::vector<char> move;
  move.push_back(i);
  move.push_back(j);
  move.push_back(type);
  return move;
}

bool Game::is_check_m() {
  char x, y;
  for (auto &v : m_pieces) {
    if (v[2] == 7) {  // my king position
      x = v[0];
      y = v[1];
      break;
    }
  }

  // true - check
  // false - king is safe
  int valid = check_check(x, y, m_board);
  if (valid != 0) return true;
  return false;
}

bool Game::is_check_e() {
  char x, y;
  for (auto &v : e_pieces) {
    if (v[2] == 7) {  // my king position
      x = v[0];
      y = v[1];
      break;
    }
  }

  // true - check
  // false - king is safe
  int valid = check_check(x, y, e_board);
  if (valid != 0) return true;
  return false;
}

// 0 - safe king
// 1 - king in danger
int check_check(char i, char j, std::vector<std::vector<char>> &chess_board) {
  // check pawn attack
  int cod_piesa = chess_board[i + 1][j + 1];
  if (cod_piesa == 11) return 1;
  cod_piesa = chess_board[i + 1][j - 1];
  if (cod_piesa == 11) return 1;

  // check knight attack
  int temp = check_knight(i, j, chess_board);
  if (temp == 1) return 1;

  for(auto d : all_directions) {
    char x = d.first;
    char y = d.second;
    while (true) {
      temp = check_check_validity(i + x, j + y, chess_board);
      if (temp == 0) return 0; // free way or out of board
      if (temp == 1) return 0; // one of my pieces
      if (temp == 2) {  // enemy piece
        cod_piesa = chess_board[i + x][j + y];
        // check diagonal for queen and bishop attack
        if ((d == ur || d == ul || d == dr || d == dl) && (cod_piesa == 14 || cod_piesa == 15 || cod_piesa == 16)) return 1;
        // check lines for queen and rook attack
        if ((d == up || d == dw || d == rg || d == lf) && (cod_piesa == 12 || cod_piesa == 16)) return 1;
        return 0;
      }
      i += x;
      j += y;
    }
  }
  return 0;
}

// 0 - safe king
// 1 - king in danger
int check_knight(char i, char j, std::vector<std::vector<char>> &chess_board) {
  int cod_piesa = chess_board[i + 1][j + 2];
  if (check_validity(i + 1, j + 2, chess_board) == 2 && cod_piesa == 13) return 1;
  cod_piesa = chess_board[i + 2][j + 1];
  if (check_validity(i + 2, j + 1, chess_board) == 2 && cod_piesa == 13) return 1;
  cod_piesa = chess_board[i + 1][j - 2];
  if (check_validity(i + 1, j - 2, chess_board) == 2 && cod_piesa == 13) return 1;
  cod_piesa = chess_board[i + 2][j - 1];
  if (check_validity(i + 2, j - 1, chess_board) == 2 && cod_piesa == 13) return 1;
  cod_piesa = chess_board[i - 1][j + 2];
  if (check_validity(i - 1, j + 2, chess_board) == 2 && cod_piesa == 13) return 1;
  cod_piesa = chess_board[i - 2][j + 1];
  if (check_validity(i - 2, j + 1, chess_board) == 2 && cod_piesa == 13) return 1;
  cod_piesa = chess_board[i - 1][j - 2];
  if (check_validity(i - 1, j - 2, chess_board) == 2 && cod_piesa == 13) return 1;
  cod_piesa = chess_board[i - 2][j - 1];
  if (check_validity(i - 2, j - 1, chess_board) == 2 && cod_piesa == 13) return 1;
  return 0;
}

// 0 - out of board
// 1 - my pieces
// 2 - enemy pieces
// 3 - free place
int check_check_validity(char i, char j, std::vector<std::vector<char>> &chess_board) {
      if (i < 0 || i >= BOX_LENGTH || j < 0 || j >= BOX_LENGTH) return 0; // out of board
      if (chess_board[i][j] > 10) return 2; // enemy pieces
      if (chess_board[i][j] > 0) return 1; // my pieces
      return 3;
}

void Game::remake_print_board(char wb) {
  m_color = wb;
  make_print_board_matrix(*this, wb);
}
