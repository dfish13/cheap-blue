#!/usr/bin/env python3
"""
Play matches between different versions of Cheap Blue to measure improvement.
Requires python-chess: pip install chess
"""

import argparse
import sys
import subprocess
import json
import os
from datetime import datetime
from pathlib import Path

try:
    import chess
    import chess.engine
except ImportError:
    print("Error: python-chess is not installed")
    print("Install it with: pip install chess")
    sys.exit(1)


class MatchRunner:
    def __init__(self, engine1_path, engine2_path, games=10, time_per_move=1000,
                 openings_file=None, verbose=False):
        self.engine1_path = engine1_path
        self.engine2_path = engine2_path
        self.games = games
        self.time_per_move = time_per_move
        self.openings_file = openings_file
        self.verbose = verbose
        self.openings = []

        if openings_file:
            self._load_openings()

    def _load_openings(self):
        """Load opening positions from file (one FEN per line)"""
        try:
            with open(self.openings_file, 'r') as f:
                self.openings = [line.strip() for line in f if line.strip() and not line.startswith('#')]
            print(f"Loaded {len(self.openings)} opening positions")
        except FileNotFoundError:
            print(f"Warning: Openings file '{self.openings_file}' not found. Using start position.")

    def _get_opening(self, game_num):
        """Get opening position for a game"""
        if not self.openings:
            return chess.STARTING_FEN
        return self.openings[game_num % len(self.openings)]

    def play_game(self, white_engine_path, black_engine_path, game_num):
        """Play a single game between two engines"""
        board = chess.Board(self._get_opening(game_num))

        if self.verbose:
            print(f"\nGame {game_num + 1}: White={Path(white_engine_path).name} vs Black={Path(black_engine_path).name}")
            print(f"Starting position: {board.fen()}")

        engines = {}
        try:
            # Start engines
            engines['white'] = chess.engine.SimpleEngine.popen_uci(white_engine_path)
            engines['black'] = chess.engine.SimpleEngine.popen_uci(black_engine_path)

            move_count = 0
            max_moves = 200  # Draw by move limit

            while not board.is_game_over() and move_count < max_moves:
                current_engine = engines['white'] if board.turn == chess.WHITE else engines['black']

                # Get move from engine
                result = current_engine.play(
                    board,
                    chess.engine.Limit(time=self.time_per_move / 1000.0)
                )

                if self.verbose:
                    print(f"{move_count + 1}. {result.move}", end=" ")
                    if move_count % 2 == 1:
                        print()

                board.push(result.move)
                move_count += 1

            # Determine result
            if board.is_checkmate():
                winner = 'black' if board.turn == chess.WHITE else 'white'
                result = 'white' if winner == 'white' else 'black'
            elif board.is_stalemate() or board.is_insufficient_material() or \
                 board.is_fifty_moves() or board.is_repetition() or move_count >= max_moves:
                result = 'draw'
            else:
                result = 'draw'

            if self.verbose:
                print(f"\nResult: {result}")
                print(f"Final position: {board.fen()}")

            return result, move_count

        finally:
            # Clean up engines
            for engine in engines.values():
                try:
                    engine.quit()
                except:
                    pass

    def run_match(self):
        """Run a complete match with alternating colors"""
        results = {
            'engine1_wins': 0,
            'engine2_wins': 0,
            'draws': 0,
            'games': []
        }

        print(f"\nStarting match: {self.games} games")
        print(f"Engine 1: {self.engine1_path}")
        print(f"Engine 2: {self.engine2_path}")
        print(f"Time per move: {self.time_per_move}ms")
        print("=" * 60)

        for game_num in range(self.games):
            # Alternate colors
            if game_num % 2 == 0:
                white_engine = self.engine1_path
                black_engine = self.engine2_path
                white_is_engine1 = True
            else:
                white_engine = self.engine2_path
                black_engine = self.engine1_path
                white_is_engine1 = False

            try:
                result, moves = self.play_game(white_engine, black_engine, game_num)

                # Record result from engine1's perspective
                if result == 'draw':
                    results['draws'] += 1
                    game_result = 'draw'
                elif result == 'white':
                    if white_is_engine1:
                        results['engine1_wins'] += 1
                        game_result = 'engine1'
                    else:
                        results['engine2_wins'] += 1
                        game_result = 'engine2'
                else:  # black wins
                    if white_is_engine1:
                        results['engine2_wins'] += 1
                        game_result = 'engine2'
                    else:
                        results['engine1_wins'] += 1
                        game_result = 'engine1'

                results['games'].append({
                    'game': game_num + 1,
                    'white': 'engine1' if white_is_engine1 else 'engine2',
                    'result': game_result,
                    'moves': moves
                })

                print(f"Game {game_num + 1}/{self.games}: {game_result} ({moves} moves) | "
                      f"Score: {results['engine1_wins']}-{results['engine2_wins']}-{results['draws']}")

            except Exception as e:
                print(f"Error in game {game_num + 1}: {e}")
                continue

        return results

    def print_summary(self, results):
        """Print match summary"""
        total = results['engine1_wins'] + results['engine2_wins'] + results['draws']

        print("\n" + "=" * 60)
        print("MATCH SUMMARY")
        print("=" * 60)
        print(f"Engine 1: {Path(self.engine1_path).name}")
        print(f"Engine 2: {Path(self.engine2_path).name}")
        print(f"\nResults ({total} games):")
        print(f"  Engine 1 wins: {results['engine1_wins']} ({results['engine1_wins']/total*100:.1f}%)")
        print(f"  Engine 2 wins: {results['engine2_wins']} ({results['engine2_wins']/total*100:.1f}%)")
        print(f"  Draws:         {results['draws']} ({results['draws']/total*100:.1f}%)")

        # Calculate score (win=1, draw=0.5, loss=0)
        engine1_score = results['engine1_wins'] + results['draws'] * 0.5
        engine2_score = results['engine2_wins'] + results['draws'] * 0.5

        print(f"\nScore: {engine1_score} - {engine2_score}")

        if engine1_score > engine2_score:
            diff = engine1_score - engine2_score
            print(f"Winner: Engine 1 (+{diff:.1f})")
        elif engine2_score > engine1_score:
            diff = engine2_score - engine1_score
            print(f"Winner: Engine 2 (+{diff:.1f})")
        else:
            print("Result: Tied")

    def save_results(self, results, output_file):
        """Save results to JSON file"""
        data = {
            'timestamp': datetime.now().isoformat(),
            'engine1': str(self.engine1_path),
            'engine2': str(self.engine2_path),
            'games': self.games,
            'time_per_move': self.time_per_move,
            'results': results
        }

        with open(output_file, 'w') as f:
            json.dump(data, f, indent=2)

        print(f"\nResults saved to: {output_file}")


def main():
    parser = argparse.ArgumentParser(
        description='Play matches between different versions of Cheap Blue'
    )
    parser.add_argument('engine1', help='Path to first engine (or version name)')
    parser.add_argument('engine2', help='Path to second engine (or version name)')
    parser.add_argument('-n', '--games', type=int, default=10,
                       help='Number of games to play (default: 10)')
    parser.add_argument('-t', '--time', type=int, default=1000,
                       help='Time per move in milliseconds (default: 1000)')
    parser.add_argument('-o', '--openings', type=str,
                       help='File with opening positions (one FEN per line)')
    parser.add_argument('-s', '--save', type=str,
                       help='Save results to JSON file')
    parser.add_argument('-v', '--verbose', action='store_true',
                       help='Verbose output (show all moves)')

    args = parser.parse_args()

    # Resolve engine paths (check if it's a version name or direct path)
    def resolve_engine_path(engine_arg):
        if os.path.exists(engine_arg):
            return engine_arg
        # Check in versions directory
        version_path = f"versions/{engine_arg}"
        if os.path.exists(version_path):
            return version_path
        # Check if it's the current binary
        if engine_arg in ['current', 'bin/uci']:
            if os.path.exists('bin/uci'):
                return 'bin/uci'
        print(f"Error: Engine not found: {engine_arg}")
        sys.exit(1)

    engine1_path = resolve_engine_path(args.engine1)
    engine2_path = resolve_engine_path(args.engine2)

    # Run match
    runner = MatchRunner(
        engine1_path,
        engine2_path,
        games=args.games,
        time_per_move=args.time,
        openings_file=args.openings,
        verbose=args.verbose
    )

    results = runner.run_match()
    runner.print_summary(results)

    if args.save:
        runner.save_results(results, args.save)


if __name__ == '__main__':
    main()
