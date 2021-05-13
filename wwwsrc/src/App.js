import React from "react";
import {
  BrowserRouter as Router,
  Switch,
  Route,
  Link
} from "react-router-dom";

import 'bootstrap/dist/css/bootstrap.min.css';

// TODO: I'm not yet sure if this is really required
//import 'bootstrap/dist/js/bootstrap.min.js';

export default function App() {
  return (
    <Router>
      <div>
        <nav class="navbar navbar-expand-lg navbar-light bg-light">
          <span class="navbar-brand">ICON</span>
          <ul class="navbar-nav mr-auto">
            <li class="nav-item active">
              <Link to="/" class="nav-link">Home</Link>
            </li>
            <li class="nav-item">
              <Link to="/config" class="nav-link">Config</Link>
            </li>
            <li class="nav-item">
              <Link to="/wireless" class="nav-link">Wireless</Link>
            </li>
          </ul>
        </nav>

        {/* A <Switch> looks through its children <Route>s and
            renders the first one that matches the current URL. */}
        <Switch>
          <Route path="/config">
            <Config />
          </Route>
          <Route path="/wireless">
            <Wireless />
          </Route>
          <Route path="/">
            <Home />
          </Route>
        </Switch>
      </div>
    </Router>
  );
}

function Home() {
  return <h2>Home</h2>;
}

function Config() {
  return <h2>Config</h2>;
}

function Wireless() {
  return <h2>Wireless</h2>;
}

