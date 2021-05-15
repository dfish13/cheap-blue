import Chessboard from 'chessboardjsx';
import Chess from 'chess.js';
import { Component } from 'react';
import PropTypes from 'prop-types';

class NormalGame extends Component {
  static propTypes = { children: PropTypes.func };

  state = { fen: 'start', squareStyles: {}, pieceSquare: ''};

  componentDidMount() {
    this.game = new Chess();
  }

  onDrop = ({ sourceSquare, targetSquare }) => {
    var move = this.game.move({
      from: sourceSquare,
      to: targetSquare,
      promotion: 'q'
    });

    if (move === null) return;

    this.setState({fen: this.game.fen()});
  };

  onSquareClick = square => {
    this.setState({
      squareStyles: { [square]: { backgroundColor: 'Turquoise' } },
      pieceSquare: square
    });

    let move = this.game.move({
      from: this.state.pieceSquare,
      to: square,
      promotion: 'q'
    });

    // illegal move
    if (move === null) return;

    this.setState({ fen: this.game.fen() });
  };

  takeBack = () => {
    let move = this.game.undo();
    if (move === null) return;

    this.setState({ fen: this.game.fen() });
  } 

  render() {
    const { fen, squareStyles } = this.state;
    return this.props.children({
      position: fen,
      onDrop: this.onDrop,
      onSquareClick: this.onSquareClick,
      squareStyles,
      takeBack: this.takeBack
    });
  }
}

function NormalGameBoard() {

  return (
    <NormalGame>
        {({ position, onDrop, onSquareClick, squareStyles, takeBack }) => (
          <div>
            <button onClick={takeBack}> Take Back </button>
            <Chessboard
            calcWidth={({ screenWidth }) => (screenWidth < 500 ? 350 : 480)}
            id="normalGame"
            position={position}
            onDrop={onDrop}
            boardStyle={{
              borderRadius: '5px',
              boxShadow: `0 5px 15px rgba(0, 0, 0, 0.5)`
            }}
            onSquareClick={onSquareClick}
            squareStyles={squareStyles}
            />
          </div>
        )}
      </NormalGame>
  )
}

function TalkToServerBox() {

  const onClick = () => {console.log("Talk to Server");};

  return (
    <div>
      <button onClick={onClick}> Talk to Server </button>
      <textarea></textarea>
    </div>
  )
}

function App() {
  return (
    <div className="App">
        <NormalGameBoard/>
        <TalkToServerBox/> 
    </div>
  );
}

export default App;
