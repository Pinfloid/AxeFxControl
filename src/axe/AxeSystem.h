#pragma once

#include <Arduino.h>
#include "AxePreset.h"
#include "utility/MIDI.h"
#include "utility/elapsedMillis.h"
#include "../AxeFxControl.h"

struct Version {
	byte major, minor;
};

class AxeSystem {

	public:

		void init();
		void update();
		void refresh(bool ignoreThrottle = false);

		void sendPresetIncrement();
		void sendPresetDecrement();
		void sendSceneIncrement();
		void sendSceneDecrement();
		void sendPresetChange(const unsigned preset);
		void sendSceneChange(const byte newScene);

		void requestPresetDetails() { requestPresetName(); }
		void requestFirmwareVersion();		
		void requestTempo();
		void setTempo(byte tempo);
		void sendTap();
		void toggleTuner();
		void enableTuner();
		void disableTuner();

		bool isPresetChanging() { return _presetChanging; }
		bool isTunerEngaged() { return _tunerEngaged; }
		unsigned getTempo() { return _tempo; }
		AxePreset* getCurrentPreset() { return &_preset; }
		Version getFirmwareVersion() { return _firmwareVersion; }
		Version getUsbVersion() { return _usbVersion; }

		void registerConnectionStatusCallback(void (*func)(bool));
		void registerWaitingCallback(void (*func)());
		void registerPresetChangingCallback(void (*func)(int));
		void registerPresetChangeCallback(void (*func)(AxePreset *preset));
		void registerSystemChangeCallback(void (*func)());
		void registerTapTempoCallback(void (*func)());
		void registerTunerDataCallback(void (*func)(const char *, const byte, const byte));
		void registerTunerStatusCallback(void (*func)(bool));

	private:
		
		void checkFirmware();
		void checkTimers();
		void checkIncomingPreset();
		void setChanging();
		void sendSysEx(const byte length, byte *sysex);
		void setSystemConnected(bool connected);
		bool isAxeSysEx(const byte *sysex, const unsigned length);
		void parseName(const byte *sysex, byte length, byte offset, char *buffer, byte size);
		void processEffectDump(const byte *sysex, unsigned length);
		void onSystemExclusive(const byte *sysex, unsigned length);
		void onControlChange(byte channel, byte number, byte value);
		void onProgramChange(byte channel, byte value);

		void requestPresetName(int preset = -1);
		void requestSceneName();
		void requestSceneNumber();
		void requestEffectDetails();

		void callConnectionStatusCallback(bool connected);	
		void callTapTempoCallback();
		void callPresetChangingCallback(int presetNumber);
		void callPresetChangeCallback(AxePreset *preset);
		void callSystemChangeCallback();
		void callTunerDataCallback(const char *note, const byte string, const byte fineTune);
		void callTunerStatusCallback(bool enabled);

		bool isValidPresetNumber(int preset);
		bool isValidSceneNumber(int scene); 

		AxePreset _preset, _incomingPreset;
		Version _firmwareVersion;
		Version _usbVersion;
		byte _tempo;
		byte _bank;
		byte _midiChannel = 1; //TODO: ask Axe
		bool _firmwareRequested = false;
		bool _tunerEngaged = false;
		bool _systemConnected = false;
		bool _presetChanging = false;
		elapsedMillis _lastSysexResponse, _lastTunerResponse, _lastRefresh;

		void (*_connectionStatusCallback)(bool);
		void (*_tapTempoCallback)();
		void (*_presetChangingCallback)(const int);
		void (*_presetChangeCallback)(AxePreset*);
		void (*_systemChangeCallback)();
		void (*_tunerStatusCallback)(bool);
		void (*_tunerDataCallback)(const char *, const byte, const byte);

	public:

		const static byte BANK_SIZE 												= 128;
		const static byte MAX_BANKS 												= 4;
		const static byte MAX_SCENES 												= 8;
		const static byte TEMPO_MIN 												= 24;
		const static byte TEMPO_MAX 												= 250;
		constexpr static byte MAX_PRESETS 									= (byte) (MAX_BANKS * BANK_SIZE) - 1;

	private:

		const static byte BANK_CHANGE_CC 										= 0x00;
		const static byte SYSEX_TUNER_ON 										= 0x01;
		const static byte SYSEX_TUNER_OFF 									= 0x00;

		const static byte SYSEX_MANUFACTURER_BYTE1 					= 0x00;
		const static byte SYSEX_MANUFACTURER_BYTE2 					= 0x01;
		const static byte SYSEX_MANUFACTURER_BYTE3 					= 0x74;
		const static byte SYSEX_AXE_VERSION 								= 0x10;
		const static byte SYSEX_QUERY_BYTE 									= 0x7F;
		const static byte SYSEX_CHECKSUM_PLACEHOLDER 				= 0x00;

		const static byte SYSEX_REQUEST_FIRMWARE 						= 0x08;
		const static byte SYSEX_REQUEST_PRESET_INFO 				= 0x0D;
		const static byte SYSEX_REQUEST_SCENE_INFO 					= 0x0E;
		const static byte SYSEX_REQUEST_SCENE_NUMBER 				= 0x0C;
		const static byte SYSEX_TAP_TEMPO_PULSE 						= 0x10;
		const static byte SYSEX_TUNER 											= 0x11;
		const static byte SYSEX_EFFECT_DUMP 								= 0x13;
		const static byte SYSEX_REQUEST_TEMPO 							= 0x14;

		const char *_notes[12] = {"A ", "Bb", "B ", "C ", "C#", "D ", "Eb", "E ", "F ", "F#", "G ", "Ab"};

		const byte REQUEST_FIRMWARE_VERSION_COMMAND_6_BYTES[6] = {
				SYSEX_MANUFACTURER_BYTE1, SYSEX_MANUFACTURER_BYTE2,
				SYSEX_MANUFACTURER_BYTE3, SYSEX_AXE_VERSION,
				SYSEX_REQUEST_FIRMWARE,   SYSEX_CHECKSUM_PLACEHOLDER};

		const byte REQUEST_EFFECT_DUMP_6_BYTES[6] = {
				SYSEX_MANUFACTURER_BYTE1, SYSEX_MANUFACTURER_BYTE2,
				SYSEX_MANUFACTURER_BYTE3, SYSEX_AXE_VERSION,
				SYSEX_EFFECT_DUMP,        SYSEX_CHECKSUM_PLACEHOLDER};

		const byte REQUEST_PRESET_NAME_COMMAND_8_BYTES[8] = {
				SYSEX_MANUFACTURER_BYTE1,  SYSEX_MANUFACTURER_BYTE2,
				SYSEX_MANUFACTURER_BYTE3,  SYSEX_AXE_VERSION,
				SYSEX_REQUEST_PRESET_INFO, SYSEX_QUERY_BYTE,
				SYSEX_QUERY_BYTE,          SYSEX_CHECKSUM_PLACEHOLDER};

		const byte REQUEST_SCENE_NAME_COMMAND_7_BYTES[7] = {
				SYSEX_MANUFACTURER_BYTE1,  SYSEX_MANUFACTURER_BYTE2,
				SYSEX_MANUFACTURER_BYTE3,  SYSEX_AXE_VERSION,
				SYSEX_REQUEST_SCENE_INFO,  SYSEX_QUERY_BYTE,
				SYSEX_CHECKSUM_PLACEHOLDER};

		const byte REQUEST_SCENE_NUMBER_COMMAND_7_BYTES[7] = {
				SYSEX_MANUFACTURER_BYTE1,   SYSEX_MANUFACTURER_BYTE2,
				SYSEX_MANUFACTURER_BYTE3,   SYSEX_AXE_VERSION,
				SYSEX_REQUEST_SCENE_NUMBER, SYSEX_QUERY_BYTE,
				SYSEX_CHECKSUM_PLACEHOLDER};

		const byte REQUEST_TEMPO_COMMAND_8_BYTES[8] = {
				SYSEX_MANUFACTURER_BYTE1, SYSEX_MANUFACTURER_BYTE2,
				SYSEX_MANUFACTURER_BYTE3, SYSEX_AXE_VERSION,
				SYSEX_REQUEST_TEMPO,      SYSEX_QUERY_BYTE,
				SYSEX_QUERY_BYTE,         SYSEX_CHECKSUM_PLACEHOLDER};

		const byte TAP_TEMPO_PULSE_COMMAND_6_BYTES[6] = {
				SYSEX_MANUFACTURER_BYTE1, SYSEX_MANUFACTURER_BYTE2,
				SYSEX_MANUFACTURER_BYTE3, SYSEX_AXE_VERSION,
				SYSEX_TAP_TEMPO_PULSE,    SYSEX_CHECKSUM_PLACEHOLDER};

		const byte ENABLE_TUNER_COMMAND_7_BYTES[7] = {
				SYSEX_MANUFACTURER_BYTE1,	SYSEX_MANUFACTURER_BYTE2,
				SYSEX_MANUFACTURER_BYTE3,	SYSEX_AXE_VERSION,
				SYSEX_TUNER,							SYSEX_TUNER_ON,
				SYSEX_CHECKSUM_PLACEHOLDER};

		const byte DISABLE_TUNER_COMMAND_7_BYTES[7] = {
				SYSEX_MANUFACTURER_BYTE1,	SYSEX_MANUFACTURER_BYTE2,
				SYSEX_MANUFACTURER_BYTE3,	SYSEX_AXE_VERSION,
				SYSEX_TUNER,							SYSEX_TUNER_OFF,
				SYSEX_CHECKSUM_PLACEHOLDER};

};