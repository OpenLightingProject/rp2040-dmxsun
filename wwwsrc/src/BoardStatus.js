import React from 'react';

import Slider from 'react-input-slider';
import * as Icon from 'react-bootstrap-icons';

class BoardStatus extends React.Component{
    constructor() {
        super();

        this.setBrightnessTimeout = undefined;

        this.modalBoardName = this.modalBoardName.bind(this);
        this.modalOwnIp = this.modalOwnIp.bind(this);
    }

    componentDidMount() {
        // Initialize all tooltips
        var tooltipTriggerList = [].slice.call(document.querySelectorAll('[data-bs-toggle="tooltip"]'))
        tooltipTriggerList.map(function (tooltipTriggerEl) {
            return new window.bootstrap.Tooltip(tooltipTriggerEl)
        });

        // Focus the modals' input fields when they open
        var myModal;

        myModal = document.getElementById('modalBoardName')
        if (myModal) {
            myModal.addEventListener('shown.bs.modal', () => {
                document.getElementById('modalBoardNameInput').value = this.props.config.boardName;
                document.getElementById('modalBoardNameInput').focus();
            });
        }

        myModal = document.getElementById('modalOwnIp')
        if (myModal) {
            myModal.addEventListener('shown.bs.modal', () => {
                document.getElementById('modalOwnIpInput').value = this.props.config.ownIp;
                document.getElementById('modalOwnIpInput').focus();
            });
        }

    }

    setStatusLedBrightness(newValue) {
        // We do the writing behind a "timeout" so we only write
        // once every 100ms in order not to overload the device
        if (this.setBrightnessTimeout) {
            window.clearTimeout(this.setBrightnessTimeout);
        }
        this.setBrightnessTimeout = window.setTimeout(this.realSetStatusLedBrightness.bind(this), 100, newValue);
    }

    realSetStatusLedBrightness(newValue) {
        const url = window.urlPrefix + '/config/statusLeds/brightness/set.json?value=' + newValue;
        fetch(url).finally(() => { this.props.updateOverview() })
    }

    loadConfig(slot, e) {
        console.log('Loading configuration from slot ' +  slot, e.target);

        const url = window.urlPrefix + '/config/load.json?slot=' + slot;
        fetch(url)
            .then(res => res.json())
            .catch(
                () => {
                    e.target.className = "btn btn-danger";
                    window.setTimeout(() => {e.target.className = "btn btn-outline-secondary"}, 3000);
                }
            )
            .then(
                (result) => {
                    if (result) {
                        console.log('Overview fetched: ', result);
                        e.target.className = "btn btn-success";
                        window.setTimeout(() => {e.target.className = "btn btn-outline-secondary"}, 3000);
                    }
                }
            )
            .finally(() => { this.props.updateOverview() })
    }

    saveConfig(slot, e) {
        console.log('Saving configuration to slot ' +  slot, e.target);

        const url = window.urlPrefix + '/config/save.json?slot=' + slot;
        fetch(url)
            .then(res => res.json())
            .catch(
                () => {
                    e.target.className = "btn btn-danger";
                    window.setTimeout(() => {e.target.className = "btn btn-outline-secondary"}, 3000);
                }
            )
            .then(
                (result) => {
                    if (result) {
                        console.log('Overview fetched: ', result);
                        e.target.className = "btn btn-success";
                        window.setTimeout(() => {e.target.className = "btn btn-outline-secondary"}, 3000);
                    }
                }
            );
    }

    enableConfig(slot, e) {
        console.log('enableConfig configuration to slot ', slot);

        const url = window.urlPrefix + '/config/enable.json?slot=' + slot;
        fetch(url)
            .then(res => res.json())
            .catch(
                () => {
                    e.target.className = "btn btn-danger";
                    window.setTimeout(() => {e.target.className = "btn btn-outline-secondary"}, 3000);
                }
            )
            .then(
                (result) => {
                    if (result) {
                        console.log('Overview fetched: ', result);
                        e.target.className = "btn btn-success";
                        window.setTimeout(() => {e.target.className = "btn btn-outline-secondary"}, 3000);
                    }
                }
            );
    }

    disableConfig(slot, e) {
        console.log('disableConfig configuration to slot ', slot);

        const url = window.urlPrefix + '/config/disable.json?slot=' + slot;
        fetch(url)
            .then(res => res.json())
            .catch(
                () => {
                    e.target.className = "btn btn-danger";
                    window.setTimeout(() => {e.target.className = "btn btn-outline-secondary"}, 3000);
                }
            )
            .then(
                (result) => {
                    if (result) {
                        console.log('Overview fetched: ', result);
                        e.target.className = "btn btn-success";
                        window.setTimeout(() => {e.target.className = "btn btn-outline-secondary"}, 3000);
                    }
                }
            );
    }

    handleModalInput(e) {
        let modalName = e.currentTarget.parentNode.parentNode.parentNode.parentNode.id;
        let paramName = modalName.substring(5);
        let newValue = document.getElementById(modalName + 'Input').value;

        const url = window.urlPrefix + '/config/set.json?' + paramName + '=' + encodeURIComponent(newValue);
        fetch(url)
            .then(res => res.json())
            .catch(
                () => {
                    e.target.className = "btn btn-danger";
                    window.setTimeout(() => {e.target.className = "btn btn-outline-secondary"}, 3000);
                }
            )
            .then(
                (result) => {
                    if (result) {
                        console.log('Overview fetched: ', result);
                        e.target.className = "btn btn-success";
                        window.setTimeout(() => {e.target.className = "btn btn-outline-secondary"}, 3000);
                    }
                }
            )
            .finally(() => { this.props.updateOverview() })
    }

    modalBoardName() {
        // The modals used to have the className "fade" as well. However, this
        // broke the "static" backdrop and the modals closed when clicked, even
        // if they shouldn't. Maybe a bootstrap-bug?
        return(
            <div className="modal" id="modalBoardName" data-bs-backdrop="static" data-bs-keyboard="false" tabIndex="-1" aria-labelledby="modalBoardNameLabel" aria-hidden="true">
                <div className="modal-dialog">
                    <div className="modal-content">
                        <div className="modal-header">
                            <h5 className="modal-title" id="modalBoardNameLabel">Edit board name</h5>
                            <button type="button" className="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                        </div>
                        <div className="modal-body">
                            <input type="text" className="form-control" id="modalBoardNameInput" />
                        </div>
                        <div className="modal-footer">
                            <button type="button" className="btn btn-secondary" data-bs-dismiss="modal">Close</button>
                            <button type="button" className="btn btn-primary" onClick={this.handleModalInput.bind(this)}>Save changes</button>
                        </div>
                    </div>
                </div>
            </div>
        )
    }

    modalOwnIp() {
        // The modals used to have the className "fade" as well. However, this
        // broke the "static" backdrop and the modals closed when clicked, even
        // if they shouldn't. Maybe a bootstrap-bug?
        return(
            <div className="modal" id="modalOwnIp" data-bs-backdrop="static" data-bs-keyboard="false" tabIndex="-1" aria-labelledby="modalOwnIpLabel" aria-hidden="true">
                <div className="modal-dialog">
                    <div className="modal-content">
                        <div className="modal-header">
                            <h5 className="modal-title" id="modalOwnIpLabel">Edit board's IP address</h5>
                            <button type="button" className="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                        </div>
                        <div className="modal-body">
                            <input type="text" className="form-control" id="modalOwnIpInput" />
                        </div>
                        <div className="modal-footer">
                            <button type="button" className="btn btn-secondary" data-bs-dismiss="modal">Close</button>
                            <button type="button" className="btn btn-primary" onClick={this.handleModalInput.bind(this)}>Save changes</button>
                        </div>
                    </div>
                </div>
            </div>
        )
    }

  render() {
    return(
        <table className="table" style={{ padding: '0px' }}>
            <tbody>
                <tr style={{ padding: '0px' }}>
                    <td colSpan="16" style={{ textAlign: 'left', border: '2px solid black' }} >

                       <table className="table">
                            <tbody>
                                <tr>
                                    <td style={{ fontWeight: 'bold' }}>Board Name:</td>
                                    <td>
                                        { this.props.withEdit &&
                                        <span data-bs-toggle="modal" data-bs-target="#modalBoardName">
                                            <this.modalBoardName />
                                            <button type="button" className="btn btn-outline-secondary p-1 m-1"
                                            data-bs-toggle="tooltip" data-bs-placement="bottom"
                                            title="Edit">
                                                <Icon.Pencil width={24} height={24} pointerEvents="none"/>
                                            </button>
                                        </span> }
                                        {this.props.config.boardName}
                                    </td>

                                    <td style={{ fontWeight: 'bold' }}>Dongle IP address:</td>
                                    <td>
                                        { this.props.withEdit &&
                                        <span data-bs-toggle="modal" data-bs-target="#modalOwnIp">
                                            <this.modalOwnIp />
                                            <button type="button" className="btn btn-outline-secondary p-1 m-1"
                                            data-bs-toggle="tooltip" data-bs-placement="bottom"
                                            title="Edit">
                                                <Icon.Pencil width={24} height={24} pointerEvents="none"/>
                                            </button>
                                        </span> }
                                        {this.props.config.ownIp}
                                    </td>

                                    <td style={{ fontWeight: 'bold' }}>Serial number:</td>
                                    <td>{this.props.config.serial}</td>
                                </tr>
                                <tr>
                                    <td style={{ fontWeight: 'bold' }}>Config source:</td>
                                    <td>{this.props.config.configSource}</td>

                                    <td style={{ fontWeight: 'bold' }}>IP address assigned to host:</td>
                                    <td>{this.props.config.hostIp}</td>

                                    <td style={{ fontWeight: 'bold' }}>Wireless module available:</td>
                                    <td>{ this.props.config.wirelessModule ? <Icon.CheckSquare width={32} height={32} /> : <Icon.XSquareFill width={32} height={32} /> }</td>
                                </tr>
                                <tr>
                                    <td style={{ fontWeight: 'bold' }}>Firmware version:</td>
                                    <td>{this.props.config.version}</td>

                                    <td style={{ fontWeight: 'bold' }}>Network mask:</td>
                                    <td>{this.props.config.ownMask}</td>

                                    <td  style={{ fontWeight: 'bold' }}>Status LED brightness:</td>
                                    <td>
                                        {this.props.config.statusLedBrightness}
                                        &nbsp;&nbsp;
                                        <Slider
                                            axis="x"
                                            xstep={10}
                                            xmin={1}
                                            xmax={255}
                                            x={this.props.config.statusLedBrightness}
                                            onChange={({ x }) => this.setStatusLedBrightness(x)}
                                        />
                                    </td>
                                </tr>
                                {this.props.withEdit && 
                                <tr>
                                    <td colSpan="6" className="text-center">
                                        Manage config on base board:&nbsp;&nbsp;&nbsp;
                                        <button type="button" className="btn btn-outline-secondary"
                                          data-bs-toggle="tooltip" data-bs-placement="bottom"
                                          title="Load configuration from base board"
                                          onClick={this.loadConfig.bind(this, 4)}>
                                            <Icon.JournalArrowUp width={32} height={32} pointerEvents="none"/>
                                        </button>
                                        &nbsp;
                                        <button type="button" className="btn btn-outline-secondary"
                                          data-bs-toggle="tooltip" data-bs-placement="bottom"
                                          title="Save configuration to base board"
                                          onClick={this.saveConfig.bind(this, 4)}>
                                            <Icon.JournalArrowDown width={32} height={32} pointerEvents="none"/>
                                        </button>
                                        &nbsp;
                                        <button type="button" className="btn btn-outline-secondary"
                                          data-bs-toggle="tooltip" data-bs-placement="bottom"
                                          title="Enable the configuration saved on the base board"
                                          onClick={this.enableConfig.bind(this, 4)}>
                                              <Icon.JournalCheck width={32} height={32} pointerEvents="none"/>
                                        </button>
                                        &nbsp;
                                        <button type="button" className="btn btn-outline-secondary"
                                          data-bs-toggle="tooltip" data-bs-placement="bottom"
                                          title="Disable the configuration saved on the base board"
                                          onClick={this.disableConfig.bind(this, 4)}>
                                              <Icon.JournalX width={32} height={32} pointerEvents="none"/>
                                        </button>
                                    </td>
                                </tr>
                                }
                            </tbody>
                        </table>
                    </td>
                </tr>
                <tr>
                    {[...Array(4)].map((value, slot) => {
                        return (
                            <td key={slot} colSpan="4" style={{ textAlign: 'center', border: '2px solid black' }}>
                                Slot&nbsp;{slot} { this.props.ioBoards.boards[slot].exist ? <Icon.CheckSquare width={32} height={32} /> : <Icon.XSquareFill width={32} height={32} /> }<br />
                                Type: { this.props.ioBoards.boards[slot].exist ? this.props.ioBoards.boards[slot].type : '???' }<br />
                                {this.props.withEdit &&
                                <div>
                                    <button type="button" className="btn btn-outline-secondary"
                                      data-bs-toggle="tooltip" data-bs-placement="bottom"
                                      title="Load configuration from this io board"
                                      onClick={this.saveConfig.bind(this, slot)}>
                                        <Icon.JournalArrowUp width={32} height={32} pointerEvents="none"/>
                                    </button>
                                    &nbsp;
                                    <button type="button" className="btn btn-outline-secondary"
                                      data-bs-toggle="tooltip" data-bs-placement="bottom"
                                      title="Save configuration to this io board"
                                      onClick={this.saveConfig.bind(this, slot)}>
                                        <Icon.JournalArrowDown width={32} height={32} pointerEvents="none"/>
                                    </button>
                                    &nbsp;
                                    <button type="button" className="btn btn-outline-secondary"
                                      data-bs-toggle="tooltip" data-bs-placement="bottom"
                                      title="Enable the configuration saved on this io board"
                                      onClick={this.enableConfig.bind(this, slot)}>
                                          <Icon.JournalCheck width={32} height={32} pointerEvents="none"/>
                                    </button>
                                    &nbsp;
                                    <button type="button" className="btn btn-outline-secondary"
                                      data-bs-toggle="tooltip" data-bs-placement="bottom"
                                      title="Disable the configuration saved on this io board"
                                      onClick={this.disableConfig.bind(this, slot)}>
                                          <Icon.JournalX width={32} height={32} pointerEvents="none"/>
                                    </button>
                                </div>
                                }
                            </td>
                        )
                    })}
                </tr>

                <tr>
                    {[...Array(16)].map((value, index) => {
                        let slot = Math.floor(index / 4);
                        let port = index % 4;
                        return (
                            <td key={index} style={{ textAlign: 'center', border: '2px solid black' }}>
                                { (
                                    this.props.ioBoards.boards[slot].exist &&
                                    this.props.ioBoards.boards[slot].ports[port].direction !== "unknown" &&
                                    this.props.ioBoards.boards[slot].ports[port].direction !== ""
                                    ) ?
                                <div>
                                { this.props.ioBoards.boards[slot].ports[port].connector === "xlr_5_female" && <img src="media/icon-xlr-5-female.svg" alt="icon-xlr-5-female" width={32} height={32} ></img> }
                                { this.props.ioBoards.boards[slot].ports[port].connector === "xlr_5_male" && <img src="media/icon-xlr-5-male.svg" alt="icon-xlr-5-male" width={32} height={32}></img> }
                                { this.props.ioBoards.boards[slot].ports[port].connector === "xlr_3_female" && <img src="media/icon-xlr-3-female.svg" alt="icon-xlr-3-female" width={32} height={32}></img> }
                                { this.props.ioBoards.boards[slot].ports[port].connector === "xlr_3_male" && <img src="media/icon-xlr-3-male.svg" alt="icon-xlr-3-male" width={32} height={32}></img> }
                                { this.props.ioBoards.boards[slot].ports[port].connector === "rj45" && <img src="media/icon-rj45.svg" alt="icon-rj45" width={32} height={32}></img> }
                                { this.props.ioBoards.boards[slot].ports[port].connector === "screws" && <img src="media/icon-screws.svg" alt="icon-screws" width={32} height={32}></img> }
                                <br />
                                { this.props.ioBoards.boards[slot].ports[port].direction === "out" && <Icon.ArrowDown width={32} height={32} /> }
                                { this.props.ioBoards.boards[slot].ports[port].direction === "in" && <Icon.ArrowUp width={32} height={32} /> }
                                { this.props.ioBoards.boards[slot].ports[port].direction === "switchable" && <Icon.ArrowDownUp width={32} height={32} /> }
                                </div>
                                :
                                <div><Icon.XSquareFill width={32} height={32} /></div>}
                            </td>
                        )
                    })}
                </tr>
            </tbody>
        </table>
    );
  }
}

export default BoardStatus;
