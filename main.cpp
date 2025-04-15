// #include <iostream>
// #include <fstream>
// #include <vector>

// using namespace std;

// int main() {
//     std::ifstream inputFile("testfile.mid", std::ios::binary | std::ios::in);
//     const size_t chunkSize = 1024; // Read 1KB chunks

//     if (inputFile.is_open()) {
//         std::vector<char> buffer(chunkSize);

//         while (inputFile.read(buffer.data(), chunkSize) || inputFile.gcount() > 0) {
//             cout << "file read";

//             std::streamsize bytesRead = inputFile.gcount();
//             // Process the 'bytesRead' number of bytes in the 'buffer'
//             std::cout << "Read " << bytesRead << " bytes." << std::endl;
//             // ... your processing logic for each chunk ...
//             cout << string(buffer, 4) << endl;
//         }
//         inputFile.close();
//     } else {
//         std::cerr << "Unable to open file." << std::endl;
//     }

//     return 0;
// }

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint> // For size_t
#include <iomanip>
#include <cmath>

using namespace std;

// Function to read a 4-byte integer in big-endian order
size_t readBigEndianUint32(std::ifstream& file) {
    size_t value = 0;
    file.read(reinterpret_cast<char*>(&value), 4);
    // Convert from big-endian to the system's native endianness if necessary
    // This is a simplified example and might need adjustment based on your system's endianness
    size_t result = ((value & 0xFF000000) >> 24) |
                      ((value & 0x00FF0000) >> 8)  |
                      ((value & 0x0000FF00) << 8)  |
                      ((value & 0x000000FF) << 24);
    return result;
}

struct TurnipTimeSignature {
    int length = -1;
    int numerator = -1;
    int denominator = -1;
    int clocks_per_metronome_clock = -1;
    int thirty_second_notes_per_midi_quarter_note = -1;
    bool on;
};


struct TurnipTrack {
    string track_name;
    int instrument = -1;

    string text_event;
};

struct TurnipKeySignature {
    int sharps_flats_count = -1;
    int major_minor_key = -1;
};

struct TurnipMidiFile {
    TurnipTimeSignature time_signature;
    TurnipKeySignature key_signature;

    int midi_format = -1;
    int number_of_tracks = -1;
    int ticks_per_beat = -1;
    int tempo = -1;

    vector<TurnipTrack> tracks;
};

int main() {
    std::ifstream midiFile;
    midiFile.open("testfile.mid", ios::binary | ios::in);

    TurnipMidiFile output;

    // https://www.eecs.umiboolch.edu/courses/eecs380/HANDOUTS/cppBinaryFileIO-2.html

    if (midiFile.is_open()) {
        // Read the header chunk
        char headerId[4];
        midiFile.read(headerId, 4);

        if (std::string(headerId, 4) == "MThd") {
            midiFile.seekg(5, std::ios::cur); // Skip the length section

            // Get header information
            output.midi_format = midiFile.get();
            output.number_of_tracks = midiFile.get() * 10 + midiFile.get();
            output.ticks_per_beat = midiFile.get() * 10 + midiFile.get();
            // midiFile.seekg(2, std::ios::cur); // Skip the length section

            // Now look for track chunks
            char trackId[4];
            while (midiFile.read(trackId, 4)) {
                if (std::string(trackId, 4) == "MTrk") {
                    // size_t trackLength = readBigEndianUint32(midiFile);
                    // std::cout << "Found MTrk chunk with length: " << trackLength << " bytes." << std::endl;


                    cout << midiFile.tellg() << endl;
                    // Read the track event 

                    int delta_time = 0;
                    cout << "Hello" << endl;

                    int byte;

                    bool meta_event = false;
                    vector<int> time_signature;
                    bool time_signature_on = false;
                    bool set_tempo = false;

                    vector<int> tempo_data;
                    int tempo_data_len = 0;

                    TurnipTrack current_track;

                    int track_name_size = 0;
                    bool set_track_name = false;

                    bool program_change = false;

                    bool set_key_signature = false;
                    vector<int> key_signature_data;

                    bool set_text_event = false;
                    int text_event_len = 0;

                    bool set_copyright_notice;
                    int copyright_notice_len = 0;

                    // Go through all the bytes
                    // TODO: only go through for the track length
                    while((byte = midiFile.get()) >= 0) {
                        if (time_signature_on) {
                            time_signature.push_back(byte);

                            if (time_signature.size() == 5) {
                                TurnipTimeSignature turnip_time_signature;

                                // Note: The first element in the time signature is the length of the time signature section. This is always a fixed number of 4. Idk why it's even in the midi file if it's the same every time.

                                cout << "time signature arr " << time_signature[2] << endl;

                                turnip_time_signature.numerator =                                 time_signature[1];
                                turnip_time_signature.denominator =                               pow(2, time_signature[2]);
                                turnip_time_signature.clocks_per_metronome_clock =                time_signature[3];
                                turnip_time_signature.thirty_second_notes_per_midi_quarter_note = time_signature[4];

                                output.time_signature = turnip_time_signature;

                                time_signature_on = false;

                            }

                            continue;
                        }

                        if (set_tempo) {
                            cout << "hi" << endl;
                            if (tempo_data.size() == 0 && tempo_data_len == 0) {
                                tempo_data_len = byte;

                                cout << " bob " << byte << endl;

                                continue;
                            }

                            tempo_data.push_back(byte);

                            if (tempo_data.size() >= tempo_data_len) {
                                int tempo = 0;

                                for (int i = 0; i < tempo_data_len; i++) {
                                    tempo += (tempo_data[i] * pow(256, (tempo_data_len - i - 1)));
                                }

                                // Calculate tempo
                                float tempo_f = tempo;
                                float base = 60000000;
                                cout << "tempo f " << tempo << endl;
                                output.tempo = ceil(base / tempo_f);

                                set_tempo = false;
                            }

                            continue;
                        }

                        if (set_track_name) {
                            if (byte == 0) {
                                set_track_name = false;
                                track_name_size = 0;

                                cout << "finish " << current_track.track_name << endl;

                                continue;
                            }

                            if (current_track.track_name.size() == 0 && track_name_size == 0) {
                                track_name_size = byte;

                                continue;
                            }

                            char byte_str = static_cast<char>(byte);
                            current_track.track_name += byte_str;

                            continue;
                        }

                        if (program_change) {
                            cout << "instrument " << byte << endl;
                            current_track.instrument = byte;

                            program_change = false;

                            continue;
                        }

                        if (set_key_signature) {
                            key_signature_data.push_back(byte);

                            if (key_signature_data.size() == 3) {
                                // The first byte in the key signature data is the length

                                output.key_signature.sharps_flats_count = key_signature_data[1];
                                output.key_signature.major_minor_key = key_signature_data[2];

                                vector<int> key_signature_data;
                                set_key_signature = false;

                            }

                            continue;
                        }

                        if (set_text_event) {
                            if (text_event_len == 0) {
                                text_event_len = byte;

                                continue;
                            }

                            cout << "text event len " << text_event_len << " " << current_track.text_event.size() << endl;

                            char byte_str = static_cast<char>(byte);
                            current_track.text_event += byte_str;

                            if (text_event_len <= current_track.text_event.size()) {
                                set_text_event = false;
                                cout << "Finish text event" << endl;
                            }

                            continue;
                        }

                        switch (byte) {
                            case 239: // Program Change
                                program_change = true;

                                cout << "program change" << endl;

                                break;

                            // Meta Events //
                            case 255: // Meta event
                                meta_event = true;
                                cout << "meta event triggered" << endl;

                                break;

                            case 47: // End track
                                if (meta_event) {
                                    cout << "end track" << endl;
                                    if (current_track.track_name.size() > 0) output.tracks.push_back(current_track);

                                    TurnipTrack current_track;
                                }
                                break;

                            case 88:
                                if (meta_event) {
                                    time_signature_on = true;
                                    cout << "time signature triggered" << endl;
                                }
                                meta_event = false; // Meta event has been found
                                break;
                            
                            case 89:
                                if (meta_event) {
                                    set_key_signature = true;
                                    cout << "Key signature triggered" << endl;
                                }

                                meta_event = false;
                                break;

                            case 81:
                                if (meta_event) {
                                    set_tempo = true;
                                    cout << "tempo meta event" << endl;
                                }

                                meta_event = false;
                                break;
                            
                            case 1:
                                if (meta_event) {
                                    set_text_event = true;
                                    
                                    cout << "text meta event" << endl;

                                    meta_event = false;
                                }    

                                break;
                            case 3:
                                if (meta_event) {
                                    cout << "Track name triggered" << endl;
                                    
                                    set_track_name = true;
                                }

                                meta_event = false;
                                break;

                            default:
                                break;
                        }

                    }

                } else {
                    std::cerr << "Error: Expected MTrk chunk, found: " << std::string(trackId, 4) << std::endl;
                    break;
                }
            }
        } else {
            std::cerr << "Error: Invalid MIDI header." << std::endl;
        }

        midiFile.close();
    } else {
        std::cerr << "Error: Could not open MIDI file." << std::endl;
    }

    cout << "midi format " << output.midi_format << endl;
    cout << "number of tracks " << output.number_of_tracks << endl;
    cout << "tempo " << output.tempo << endl;
    cout << "ticks per beat " << output.ticks_per_beat << endl;
    cout << "clocks per metronome clock " << output.time_signature.clocks_per_metronome_clock << endl;
    cout << "denomiator " << output.time_signature.denominator << endl;
    // cout << "length " << output.time_signature.length << endl;
    cout << "numerator " << output.time_signature.numerator << endl;
    cout << "thirty second notes per midi quarter note " << output.time_signature.thirty_second_notes_per_midi_quarter_note << endl;

    cout << "sharps or flats count " << output.key_signature.sharps_flats_count << endl;
    cout << "major or minor key " << output.key_signature.major_minor_key << endl;

    cout << "track nums " << output.tracks.size() << endl;

    for (int i = 0; i < output.tracks.size(); i++) {
        TurnipTrack track = output.tracks[i];

        cout << "track " << i << " instrument " << track.instrument << endl;
        cout << "track " << i << " track name " << track.track_name << endl;
        cout << "track " << i << " text event " << track.text_event << endl;

    }

    return 0;
}

// #include <iostream>
// #include <fstream>
// #include <string>

// int main() {
//     ofstream image;
//     image.open("image.ppm");

//     if (image.is_open()) {
//         image << "P3" << endl;
//         image << "250 250" << endl;
//         image << "255" << endl;

//         for (int y = 0; y <250; y++) {
//             for (int x = 0; x < 250; x++) {
//                 image << x << " " << y << " " << x << endl;
//             }
//         }
//     }

//     image.close();

//     return 0;
// }
