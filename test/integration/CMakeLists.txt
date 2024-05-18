# filename should be a string pointing to a MIDI file
# sha256 is the expected hash after rendering raw data
function(add_render_test filename sha256)
    add_test(
        NAME "Render ${filename}"
        COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/test_runner.py
            --render-exe $<TARGET_FILE:nuked-sc55-render>
            --input ${CMAKE_CURRENT_SOURCE_DIR}/${filename}
            --sha256 ${sha256}
            --rom-directory ${NUKED_TEST_ROMDIR}
        COMMAND_EXPAND_LISTS
    )
endfunction()

add_render_test("avmidi/01.mid" "959f4fe0d4f876f57b385bd7c292248451a5f952764506950c8f89118a7dddee")
add_render_test("avmidi/02.mid" "acb66c7665c5033c16737a921dfd7d477e1435895c5634113ea3185c71c31f50")
add_render_test("avmidi/03.mid" "0c7ce7cbe5831f7221e8448ff941218bab04040fb3426a6ea1d22c9af0e079b6")
add_render_test("avmidi/04.mid" "e98bccf3f9ce7bb228402284411837d74bf222d6194bf88d39410c7a8cf29600")
add_render_test("avmidi/05.mid" "843c595c63cf05abc4380fb245cd458829c140f4b416b73bf33c942554b6091f")
add_render_test("avmidi/06.mid" "6bf009520c4b8a88b853ffb51d262316f07e8a5ef9b5998bdf5e321e12611c5f")
add_render_test("avmidi/07.mid" "7958423b9c9e9c7b7eb0dd3f119d5fa29dfe5c0dd6d5a537bf059abf74d32b54")
add_render_test("avmidi/08.mid" "1066209eda6e63bcf4bd9c488eb5dfcec493db1d818f122bf441bc26c4e488cc")
add_render_test("avmidi/09.mid" "d2dc884a2e6ecee8d1a680340f0e3cf95cd4d2530bd2e6e078589e223874e8ec")
add_render_test("avmidi/0A.mid" "6208536e1514203eee647adc8afa433a9989a6b064369e8d07ce77da3ac0694d")
add_render_test("avmidi/0B.mid" "ef2c69af9b678e48cc0c6539cfebfaff0487474824e55c049f62c971ca4b1bc5")
add_render_test("avmidi/0C.mid" "dd5610f80314b8886eae51d90f4d1bab0bf94ac6cb486f4894c7cb0511eb7b4d")
add_render_test("avmidi/10.mid" "65e66187ea72af3f0309b2c0ed5439da79ed5a48ecf57abb6d1ce03752936c39")
add_render_test("avmidi/11.mid" "74efd0e7e4bf14c989a000dc145636701f17377c3fb6a8b6befaddc0d57ec766")
add_render_test("avmidi/12.mid" "6f16150a86f20a9fa8e75cdc2acd1227967fab88c7ccc41acc585505399778de")
add_render_test("avmidi/13.mid" "139e00db8a68b2a0ec72c71b3b6652e9edb5d57fbb8b577ec1c5b87ab77cd722")
add_render_test("avmidi/15.mid" "e8428f668a7345a4d7a223d512763a0414de01a67fe94c81bef352928fef78ef")
add_render_test("avmidi/16.mid" "e5f96f3e16e55ad27a8cdc63b6e5c04d17fa96a9f3b03321259619997572790f")
add_render_test("avmidi/17.mid" "6355e43e5162cc12d054075406bb240cede5afd198a5474f5f5eab89deb5eddb")
add_render_test("avmidi/18.mid" "626e37d8a1d578f6bcb194685157f2b908a57f44d5064aef8f51938a30d0f988")
add_render_test("avmidi/19.mid" "bff63fb9f09c2c58acfd5afacad1c9a9583da68965fef840db5f708239e39c5d")
add_render_test("avmidi/20.mid" "4fbf60ac138a49e26961962d2aa7f6aa9f9b0aa9ba1d1bde5e38ce93065cb4e0")
add_render_test("avmidi/21.mid" "193d0c6d968281ca33149d6fb2ebd4024efef6838d343daf23d05bb45adb914e")
add_render_test("avmidi/22.mid" "93da74241b7a9d475aec5eadbf800de62a04cc05259e4b4739d46c86a5952cdc")
add_render_test("avmidi/23.mid" "a52635bc4faf77a65b24ff810922a6bb6b03ab873dbf12873167bdddbfc03774")
add_render_test("avmidi/24.mid" "66cc848bbcf4822f0b19ca6fcd4f66e8d0f266052b06808ba674920cc206ebcf")
add_render_test("avmidi/25.mid" "8a70813b13f5096b9fe30ef6f99fd7ba299020091aa7ccb1f21a8bcf1f094552")
add_render_test("avmidi/25-2.mid" "760a3aa92caeaac296dc593a71da0bab7f10e71c2c5f23c61d718c574c147a6f")
add_render_test("avmidi/26.mid" "dad4c70ebb82a93fb816c75c9b3e8b47fd8d61a96e400c897fbe5385e371b3a5")
add_render_test("avmidi/27.mid" "225abc3e538d88e67cc900de68ed496ef23f5f9107a3b915ec6a882b2b622e3e")
add_render_test("avmidi/28.mid" "d734b40deabd074490ce3cff0d71c2b968015e6de38d55d81f488f1c8b04f006")
add_render_test("avmidi/29.mid" "11a27ee373ede9ba7718d43540767bdabc14169cd2d38839e23f69d73aea36c7")
add_render_test("avmidi/30.mid" "77537ed8177ee627ad1a0c7aa49bbee5d412d24b7e8f3cfc48a505e5d3057dbc")
add_render_test("avmidi/31.mid" "1289e7466c67cae745e6b52ff9b7dc60c8ebaef5cdd5b1d483188a404c1365dc")
add_render_test("avmidi/32.mid" "2e6fa3195083b9178376315361ad40d76afc5a1adac7557c36c32d6a209c0074")
