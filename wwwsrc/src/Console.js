import React from 'react';
import * as Icon from 'react-bootstrap-icons';
import Slider from 'react-input-slider';

import couch64 from 'couch64';
import snappyjs from 'snappyjs';

// TODO: Display the status to the user (up-to-date, fetching, error)

class Console extends React.Component {
    constructor() {
        super();
        this.state = {
            selectedBuffer: 0,
            updateValuesInterval: undefined,
            values: [],
            loading: false,
            channelOffset: 0,
            partyModeClicked: false,
            partyModeEnabled: false,
            partyModeBuffer: 0,
            partyModeChannel: 0,
        };
        this.setValueTimeout = undefined;

        // This binding is necessary to make `this` work in the callback
        this.selectedBufferDecrease = this.selectedBufferDecrease.bind(this);
        this.selectedBufferIncrease = this.selectedBufferIncrease.bind(this);
        this.selectedBufferFirst = this.selectedBufferFirst.bind(this);
        this.selectedBufferLast = this.selectedBufferLast.bind(this);
        this.channelOffsetDecrease = this.channelOffsetDecrease.bind(this);
        this.channelOffsetIncrease = this.channelOffsetIncrease.bind(this);
        this.channelOffsetFirst = this.channelOffsetFirst.bind(this);
        this.channelOffsetLast = this.channelOffsetLast.bind(this);
        this.setAllOff = this.setAllOff.bind(this);
        this.setAllOn = this.setAllOn.bind(this);
        this.handlePartyModeClicked = this.handlePartyModeClicked.bind(this);
        this.handleBinocularsClicked = this.handleBinocularsClicked.bind(this);
    }

    componentDidMount() {
        let interval = window.setInterval(this.updateValues.bind(this), 2000);
        this.setState({
            updateValuesInterval: interval
        });
    }

    componentWillUnmount() {
        if (this.state.updateValuesInterval) {
            window.clearInterval(this.state.updateValuesInterval);
        }
    }

    updateValues() {
        // Check if there is already a request running. If so, do nothing
        if (this.state.loading) {
            return;
        }

        this.setState({ loading: true });
        let buffer = this.state.selectedBuffer;
        if (buffer <= 9) {
            buffer = '0' + buffer;
        }
        const url = window.urlPrefix + '/dmxBuffer/' + buffer + '/get.json';
        fetch(url)
            .then(res => res.json())
            .then(
                (result) => {
                    if (result) {
                        console.log('Values fetched: ', result);

                        // TODO: Helper function to base64decode + snappy decompress
                        let compressed = couch64.base64DecToArr(result.value);
                        let uncompressed = snappyjs.uncompress(compressed);
                        let values = this.state.values;
                        values.length = 0;
                        Array.prototype.push.apply(values, new Uint8Array(uncompressed.buffer));
                        this.setState({ loading: false, values: values });
                    }
                }
            ).finally(
                () => { this.setState({ loading: false }); }
            );
    }

    setValue(channel, newValue) {
        console.log('SETVALUE CHANNEL: ' + channel + ' TO ' + newValue);
        // We do the writing behind a "timeout" so we only write
        // once every 100ms in order not to overload the device
        // TODO: Does this work when one channel's timeout is running and a request to change another one comes in?
        if (this.setValueTimeout) {
            window.clearTimeout(this.setValueTimeout);
        }
        this.setValueTimeout = window.setTimeout(this.realSetValue, 100, this.state.selectedBuffer, channel, newValue);
    }

    realSetValue(buffer, channel, newValue) {
        const url = window.urlPrefix + '/dmxBuffer/set.json?buffer=' + buffer + '&channel=' + channel + '&value=' + newValue;
        fetch(url);
    }

    selectedBufferDecrease() {
        if (this.state.selectedBuffer === 0) {
            return;
        }

        this.setState({selectedBuffer: this.state.selectedBuffer - 1});
        this.updateValues();
    }

    selectedBufferIncrease() {
        if (this.state.selectedBuffer >= 24) {
            return;
        }

        this.setState({selectedBuffer: this.state.selectedBuffer + 1});
        this.updateValues();
    }

    selectedBufferFirst() {
        this.setState({selectedBuffer: 0});
        this.updateValues();
    }

    selectedBufferLast() {
        this.setState({selectedBuffer: 24});
        this.updateValues();
    }

    channelOffsetDecrease() {
        if (this.state.channelOffset === 0) {
            return;
        }

        this.setState({channelOffset: this.state.channelOffset - 32});
    }

    channelOffsetIncrease() {
        if (this.state.channelOffset >= 470) {
            return;
        }

        this.setState({channelOffset: this.state.channelOffset + 32});
    }

    channelOffsetFirst() {
        this.setState({channelOffset: 0});
    }

    channelOffsetLast() {
        this.setState({channelOffset: 480});
    }

    setAllOff() {
        const values = new Uint8Array(512);
        // (value, start position, end position);
        values.fill(0, 0, 512);

        // TODO: Common method for snappycompress + base64encode
        const compressed = snappyjs.compress(values);
        const encoded = couch64.base64EncArr(compressed);

        const url = window.urlPrefix + '/dmxBuffer/set.json?buffer=' + this.state.selectedBuffer + '&data=' + encoded;
        fetch(url);
    }

    setAllOn() {
        const values = new Uint8Array(512);
        // (value, start position, end position);
        values.fill(255, 0, 512);

        // TODO: Common method for snappycompress + base64encode
        const compressed = snappyjs.compress(values);
        const encoded = couch64.base64EncArr(compressed);

        const url = window.urlPrefix + '/dmxBuffer/set.json?buffer=' + this.state.selectedBuffer + '&data=' + encoded;
        fetch(url);
    }

    handlePartyModeClicked(e) {
        let channel = parseInt(e.target.innerHTML) - 1;
        this.setState({partyModeClicked: true, partyModeBuffer: this.state.selectedBuffer, partyModeChannel: channel});
    }

    handleBinocularsClicked() {
        let url;
        if (this.state.partyModeEnabled) {
            url = window.urlPrefix + '/config/partyMode/set.json?enabled=0';
            this.setState({partyModeEnabled: false});
        } else {
            url = window.urlPrefix + '/config/partyMode/set.json?enabled=1&buffer=' + this.state.partyModeBuffer + '&offset=' + this.state.partyModeChannel;
            this.setState({partyModeEnabled: true});
        }
        fetch(url);
    }

    render() {
        return (
            <div className="container-fluid">
                <div className="row"><div className="col">&nbsp;</div></div>
                <div className="row"><div className="col">
                    Select buffer: &nbsp;&nbsp;
                    <Slider
                        axis="x"
                        xstep={1}
                        xmin={0}
                        xmax={24}
                        x={this.state.selectedBuffer}
                        onChange={({ x }) => this.setState({ selectedBuffer: x })}
                    />
                    &nbsp;&nbsp;
                    <Icon.ChevronBarLeft onClick={this.selectedBufferFirst} />
                    &nbsp;&nbsp;
                    <Icon.ChevronLeft onClick={this.selectedBufferDecrease} />
                    &nbsp;&nbsp;
                    { this.state.selectedBuffer }
                    &nbsp;&nbsp;
                    <Icon.ChevronRight onClick={this.selectedBufferIncrease} />
                    &nbsp;&nbsp;
                    <Icon.ChevronBarRight onClick={this.selectedBufferLast} />
                    &nbsp;&nbsp;
                    { (this.state.partyModeClicked && !this.state.partyModeEnabled) && <Icon.Binoculars onDoubleClick={this.handleBinocularsClicked} /> }
                    { (this.state.partyModeClicked && this.state.partyModeEnabled) && <Icon.BinocularsFill onDoubleClick={this.handleBinocularsClicked} /> }
                    &nbsp;&nbsp;
                    { this.state.loading && <div className="spinner-border spinner-border-sm" role="status"></div> }
                </div></div>
                <div className="row"><div className="col">&nbsp;</div></div>
                <div className="row"><div className="col">
                    Starting channel:
                    &nbsp;&nbsp;
                    <Icon.ChevronBarLeft onClick={this.channelOffsetFirst} />
                    &nbsp;&nbsp;
                    <Icon.ChevronLeft onClick={this.channelOffsetDecrease} />
                    &nbsp;&nbsp;
                    { this.state.channelOffset + 1 }
                    &nbsp;&nbsp;
                    <Icon.ChevronRight onClick={this.channelOffsetIncrease} />
                    &nbsp;&nbsp;
                    <Icon.ChevronBarRight onClick={this.channelOffsetLast} />
                    &nbsp;&nbsp;
                    All channels:
                    <Icon.LightbulbOff onClick={this.setAllOff} />
                    &nbsp;&nbsp;
                    <Icon.Lightbulb onClick={this.setAllOn} />
                </div></div>
                <div className="row"><div className="col">
                    <table className="table" style={{ padding: '0px' }}>
                        <thead>
                            <tr style={{ padding: '0px' }}>
                                {[...Array(32)].map((value, index) => {
                                    return (
                                        <th className="text-center" key={index} style={{ minWidth: '2rem', padding: '0px' }} onDoubleClick={this.handlePartyModeClicked}>{index + this.state.channelOffset + 1}</th>
                                    )
                                })}
                            </tr>
                        </thead>
                        <tbody>
                            <tr>
                                {[...Array(32)].map((value, index) => {
                                    return (
                                        <td className="text-center" key={index} style={{ padding: '0px' }}>{this.state.values[index + this.state.channelOffset]}</td>
                                    )
                                })}
                            </tr>
                            <tr>
                                {[...Array(32)].map((value, index) => {
                                    return (
                                        <td className="text-center" key={index}>
                                            <Slider
                                                axis="y"
                                                ystep={1}
                                                ymin={0}
                                                ymax={255}
                                                y={this.state.values[index + this.state.channelOffset]}
                                                yreverse={1}
                                                onChange={({ y }) => this.setValue(index + this.state.channelOffset, y)}
                                            />
                                        </td>
                                    )
                                })}
                            </tr>
                        </tbody>
                    </table>
                </div></div>
            </div>
        );
    }
}
export default Console;
