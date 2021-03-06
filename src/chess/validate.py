import sys
import subprocess
from datetime import datetime

import chess
import chess.pgn


ENGINEPATH = "../../bin/chess"
PGNPATH = "../../../chess/games/KingBase2018-A00-A39.pgn"
PGNPATH = "../../../chess/games/KingBase2018-A40-A79.pgn"; SKIP_GAMES = 2000
# Done
# PGNPATH = "../../../chess/games/KingBase2018-A80-A99.pgn"; SKIP_GAMES = 31330
PGNPATH = "../../../chess/games/KingBase2018-B00-B19.pgn"; SKIP_GAMES = 18500

engine = subprocess.Popen(
    [ENGINEPATH], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True
)

games = 0
played_moves = 0
seen_moves = 0


def print_results():
    global SKIP_GAMES, games, played_moves, seen_moves, start_time
    time_diff = datetime.now() - start_time

    print("Ran for:", time_diff)
    print("Ended on game", games)
    print(
        "Games:",
        games - SKIP_GAMES,
        "Played moves:",
        played_moves,
        "Seen moves:",
        seen_moves,
    )
    print("Moves per second:", played_moves / time_diff.total_seconds())


with open(PGNPATH, encoding="utf-8", errors="replace") as pgn_file:
    for i in range(SKIP_GAMES):
        games += 1
        try:
            game = chess.pgn.read_game(pgn_file)
        except:
            raise
    if SKIP_GAMES:
        print("Finished skipping")

    start_time = datetime.now()

    try:
        while pgn_file:
            games += 1
            try:
                game = chess.pgn.read_game(pgn_file)
            except:
                print_results()
                raise
            else:
                if game is None:
                    print(f"Finished file {PGNPATH}")
                    print_results()
                    sys.exit(0)

            board = game.board()
            for move in game.mainline_moves():
                played_moves += 1
                board.push(move)
                board.fullmove_number = 1

                engine.stdin.write(board.fen(en_passant="fen") + "\n")
                engine.stdin.flush()

                output = engine.stdout.readline()[:-1]

                engine_moves = {}
                while output != "DONE":
                    move, fen = output.split("\t")
                    assert move not in engine_moves
                    engine_moves[move] = fen
                    output = engine.stdout.readline()[:-1]
                    seen_moves += 1

                python_moves = {}
                for move in board.legal_moves:
                    board.push(move)
                    board.fullmove_number = 1
                    assert move not in python_moves

                    python_moves[move.uci()] = board.fen(en_passant="fen")
                    board.pop()

                if python_moves.keys() != engine_moves.keys():
                    if board.halfmove_clock >= 150:
                        continue
                    print_results()
                    print("--------------------------")
                    print("Different move set!")

                    print("Invalid engine moves:")
                    print(set(engine_moves.keys()) - set(python_moves.keys()))

                    print("Missing engine moves:")
                    print(set(python_moves.keys()) - set(engine_moves.keys()))

                    print("board:", f"'{board.fen(en_passant='fen')}'")
                    print("python:", python_moves.keys())
                    print("engine:", engine_moves.keys())
                    print("--------------------------")
                    sys.exit(1)

                broke_moves = []
                for move in python_moves:
                    if python_moves[move] != engine_moves[move]:
                        broke_moves.append(move)

                if len(broke_moves):
                    print_results()
                    print("--------------------------")
                    print("board:", f"'{board.fen(en_passant='fen')}'")
                    for move in broke_moves:
                        print("move:", move)
                        print("python:", python_moves[move])
                        print("engine:", engine_moves[move])
                        print()
                    print("--------------------------")
                    sys.exit(1)
    except KeyboardInterrupt:
        print_results()
        sys.exit(0)
