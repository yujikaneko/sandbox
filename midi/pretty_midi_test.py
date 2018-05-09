import pretty_midi

pm = pretty_midi.PrettyMIDI(resolution=960, initial_tempo=120)
instrument = pretty_midi.Instrument(0)

c4 = pretty_midi.note_name_to_number('C4')
e4 = pretty_midi.note_name_to_number('E4')
g4 = pretty_midi.note_name_to_number('G4')

note = pretty_midi.Note(velocity=100, pitch=g4, start=2, end=3)
instrument.notes.append(note)
note = pretty_midi.Note(velocity=100, pitch=e4, start=1, end=2)
instrument.notes.append(note)
note = pretty_midi.Note(velocity=100, pitch=c4, start=0, end=1)
instrument.notes.append(note)

pm.instruments.append(instrument)
pm.write('test.mid')


pm = pretty_midi.PrettyMIDI('MIDI_sample.mid')
print(pm.get_piano_roll())
print(pm.synthesize())
etempo = pm.estimate_tempo()
print(etempo)
delta = (60.0/etempo)
print(delta)
for note in pm.instruments[0].notes:
  print(note)

