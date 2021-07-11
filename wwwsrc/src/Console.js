import React from 'react';
import * as Icon from 'react-bootstrap-icons';
import Slider from 'react-input-slider';

import couch64 from 'couch64';
import snappyjs from 'snappyjs';

class Console extends React.Component {
    constructor() {
        super();
        this.state = {
            selectedBuffer: 0,
            updateValuesInterval: undefined,
            values: [],
            channelOffset: 0,
        };
        this.setValueTimeout = undefined;

        // This binding is necessary to make `this` work in the callback
        this.selectedBufferDecrease = this.selectedBufferDecrease.bind(this);
        this.selectedBufferIncrease = this.selectedBufferIncrease.bind(this);
        this.channelOffsetDecrease = this.channelOffsetDecrease.bind(this);
        this.channelOffsetIncrease = this.channelOffsetIncrease.bind(this);
        this.setAllOff = this.setAllOff.bind(this);
        this.setAllOn = this.setAllOn.bind(this);
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
                        this.setState({ values: values });
                    }
                }
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
        if (this.state.selectedBuffer >= 31) {
            return;
        }

        this.setState({selectedBuffer: this.state.selectedBuffer + 1});
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

    render() {
        return (
            <div className="console" class="container-fluid">
                <div class="row"><div class="col">
                    Select buffer: &nbsp;&nbsp;
                    <Slider
                        axis="x"
                        xstep={1}
                        xmin={0}
                        xmax={24}
                        x={this.state.selectedBuffer}
                        onChange={({ x }) => this.setState({ selectedBuffer: x })}
                    /> &nbsp;&nbsp; {this.state.selectedBuffer} &nbsp;&nbsp; <Icon.ChevronLeft onClick={this.selectedBufferDecrease} /> &nbsp;&nbsp; <Icon.ChevronRight onClick={this.selectedBufferIncrease} />
                </div></div>
                <div class="row"><div class="col">&nbsp;</div></div>
                <div class="row"><div class="col">
                    Starting channel: <Icon.ChevronLeft onClick={this.channelOffsetDecrease} /> &nbsp;&nbsp; <Icon.ChevronRight onClick={this.channelOffsetIncrease} /> &nbsp;&nbsp; All channels: <Icon.LightbulbOff onClick={this.setAllOff} /> &nbsp;&nbsp; <Icon.Lightbulb onClick={this.setAllOn} />
                </div></div>
                <div class="row"><div class="col">
                    <table class="table" style={{ padding: '0px' }}>
                        <thead>
                            <tr style={{ padding: '0px' }}>
                                {[...Array(32)].map((value, index) => {
                                    return (
                                        <th class="text-center" key={index} style={{ minWidth: '2rem', padding: '0px' }}>{index + this.state.channelOffset + 1}</th>
                                    )
                                })}
                            </tr>
                        </thead>
                        <tbody>
                            <tr>
                                {[...Array(32)].map((value, index) => {
                                    return (
                                        <td class="text-center" key={index} style={{ padding: '0px' }}>{this.state.values[index + this.state.channelOffset]}</td>
                                    )
                                })}
                            </tr>
                            <tr>
                                {[...Array(32)].map((value, index) => {
                                    return (
                                        <td class="text-center" key={index}>
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