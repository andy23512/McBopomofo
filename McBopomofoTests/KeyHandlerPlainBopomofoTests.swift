// Copyright (c) 2011 and onwards The McBopomofo Authors.
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

import XCTest
@testable import McBopomofo

class KeyHandlerPlainBopomofoTests: XCTestCase {
    var handler = KeyHandler()

    override func setUpWithError() throws {
        LanguageModelManager.loadDataModels()
        handler = KeyHandler()
        handler.inputMode = .plainBopomofo
    }

    override func tearDownWithError() throws {
    }

    func testPunctuationTable() {
        let input = KeyHandlerInput(inputText: "`", keyCode: 0, charCode: charCode("`"), flags: .shift, isVerticalMode: false)
        var state: InputState = InputState.Empty()
        handler.handle(input: input, state: state) { newState in
            state = newState
        } errorCallback: {
        }

        XCTAssertTrue(state is InputState.ChoosingCandidate, "\(state)")
        if let state = state as? InputState.ChoosingCandidate {
            XCTAssertTrue(state.candidates.contains("，"))
        }
    }

    func testPunctuationComma() {
        let enabled = Preferences.halfWidthPunctuationEnabled
        Preferences.halfWidthPunctuationEnabled = false
        let input = KeyHandlerInput(inputText: "<", keyCode: 0, charCode: charCode("<"), flags: .shift, isVerticalMode: false)
        var state: InputState = InputState.Empty()
        handler.handle(input: input, state: state) { newState in
            state = newState
        } errorCallback: {
        }

        XCTAssertTrue(state is InputState.ChoosingCandidate, "\(state)")
        if let state = state as? InputState.ChoosingCandidate {
            XCTAssertEqual(state.composingBuffer, "，")
        }
        Preferences.halfWidthPunctuationEnabled = enabled
    }

    func testPunctuationPeriod() {
        let enabled = Preferences.halfWidthPunctuationEnabled
        Preferences.halfWidthPunctuationEnabled = false
        let input = KeyHandlerInput(inputText: ">", keyCode: 0, charCode: charCode(">"), flags: .shift, isVerticalMode: false)
        var state: InputState = InputState.Empty()
        handler.handle(input: input, state: state) { newState in
            state = newState
        } errorCallback: {
        }

        XCTAssertTrue(state is InputState.ChoosingCandidate, "\(state)")
        if let state = state as? InputState.ChoosingCandidate {
            XCTAssertEqual(state.composingBuffer, "。")
        }
        Preferences.halfWidthPunctuationEnabled = enabled
    }

    func testInputNe() {
        let input = KeyHandlerInput(inputText: "s", keyCode: 0, charCode: charCode("s"), flags: .shift, isVerticalMode: false)
        var state: InputState = InputState.Empty()
        handler.handle(input: input, state: state) { newState in
            state = newState
        } errorCallback: {
        }

        XCTAssertTrue(state is InputState.Inputting, "\(state)")
        if let state = state as? InputState.Inputting {
            XCTAssertEqual(state.composingBuffer, "ㄋ")
        }
    }

    func testInputNi() {
        var state: InputState = InputState.Empty()
        let keys = Array("su").map {
            String($0)
        }
        for key in keys {
            let input = KeyHandlerInput(inputText: key, keyCode: 0, charCode: charCode(key), flags: [], isVerticalMode: false)
            handler.handle(input: input, state: state) { newState in
                state = newState
            } errorCallback: {
            }
        }

        XCTAssertTrue(state is InputState.Inputting, "\(state)")
        if let state = state as? InputState.Inputting {
            XCTAssertEqual(state.composingBuffer, "ㄋㄧ")
        }
    }

    func testInputNi3() {
        var state: InputState = InputState.Empty()
        let keys = Array("su3").map {
            String($0)
        }
        for key in keys {
            let input = KeyHandlerInput(inputText: key, keyCode: 0, charCode: charCode(key), flags: [], isVerticalMode: false)
            handler.handle(input: input, state: state) { newState in
                state = newState
            } errorCallback: {
            }
        }

        XCTAssertTrue(state is InputState.ChoosingCandidate, "\(state)")
        if let state = state as? InputState.ChoosingCandidate {
            XCTAssertTrue(state.candidates.contains("你"))
        }
    }

    func testCancelCandidateUsingDelete() {
        var state: InputState = InputState.Empty()
        let keys = Array("su3").map {
            String($0)
        }
        for key in keys {
            let input = KeyHandlerInput(inputText: key, keyCode: 0, charCode: charCode(key), flags: [], isVerticalMode: false)
            handler.handle(input: input, state: state) { newState in
                state = newState
            } errorCallback: {
            }
        }

        let input = KeyHandlerInput(inputText: " ", keyCode: KeyCode.delete.rawValue, charCode: 0, flags: [], isVerticalMode: false)
        handler.handle(input: input, state: state) { newState in
            state = newState
        } errorCallback: {
        }

        XCTAssertTrue(state is InputState.EmptyIgnoringPreviousState, "\(state)")
    }

    func testCancelCandidateUsingEsc() {
        var state: InputState = InputState.Empty()
        let keys = Array("su3").map {
            String($0)
        }
        for key in keys {
            let input = KeyHandlerInput(inputText: key, keyCode: 0, charCode: charCode(key), flags: [], isVerticalMode: false)
            handler.handle(input: input, state: state) { newState in
                state = newState
            } errorCallback: {
            }
        }

        let input = KeyHandlerInput(inputText: " ", keyCode: 0, charCode: 27, flags: [], isVerticalMode: false)
        handler.handle(input: input, state: state) { newState in
            state = newState
        } errorCallback: {
        }

        XCTAssertTrue(state is InputState.EmptyIgnoringPreviousState, "\(state)")
    }

    func testAssociatedPhrases() {
        let enabled = Preferences.associatedPhrasesEnabled
        Preferences.associatedPhrasesEnabled = true
        var state: InputState = InputState.Empty()
        let keys = Array("aul ").map {
            String($0)
        }
        for key in keys {
            let input = KeyHandlerInput(inputText: key, keyCode: 0, charCode: charCode(key), flags: [], isVerticalMode: false)
            handler.handle(input: input, state: state) { newState in
                state = newState
            } errorCallback: {
            }
        }

        XCTAssertTrue(state is InputState.AssociatedPhrases, "\(state)")
        if let state = state as? InputState.AssociatedPhrases {
            XCTAssertTrue(state.candidates.contains("嗚"))
        }
        Preferences.associatedPhrasesEnabled = enabled
    }


    func testNoAssociatedPhrases() {
        let enabled = Preferences.associatedPhrasesEnabled
        Preferences.associatedPhrasesEnabled = false
        var state: InputState = InputState.Empty()
        let keys = Array("aul ").map {
            String($0)
        }
        for key in keys {
            let input = KeyHandlerInput(inputText: key, keyCode: 0, charCode: charCode(key), flags: [], isVerticalMode: false)
            handler.handle(input: input, state: state) { newState in
                state = newState

            } errorCallback: {
            }
        }

        XCTAssertTrue(state is InputState.Empty, "\(state)")
        Preferences.associatedPhrasesEnabled = enabled
    }

}
